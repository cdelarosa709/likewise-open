#!/usr/bin/python
from k5test import *
import random
import re
import struct

# Convenience constants for use as expected enctypes.  defetype is the
# default enctype for master keys.
aes256 = 'aes256-cts-hmac-sha1-96'
aes128 = 'aes128-cts-hmac-sha1-96'
des3 = 'des3-cbc-sha1'
defetype = aes256

realm = K5Realm(create_host=False, start_kadmind=True)
realm.prep_kadmin()
stash_file = os.path.join(realm.testdir, 'stash')

# Count the number of principals in the realm.
nprincs = len(realm.run_kadminl('listprincs').splitlines()) - 1

# List the currently active mkeys and compare against expected
# results.  Each argument must be a sequence of four elements: an
# expected kvno, an expected enctype, whether the key is expected to
# have an activation time, and whether the key is expected to be
# currently active.
list_mkeys_re = re.compile(r'^KVNO: (\d+), Enctype: (\S+), '
                           '(Active on: [^\*]+|No activate time set)( \*)?$')
def check_mkey_list(*expected):
    # Split the output of kdb5_util list_mkeys into lines and ignore the first.
    outlines = realm.run([kdb5_util, 'list_mkeys']).splitlines()[1:]
    if len(outlines) != len(expected):
        fail('Unexpected number of list_mkeys output lines')
    for line, ex in zip(outlines, expected):
        m = list_mkeys_re.match(line)
        if not m:
            fail('Unrecognized list_mkeys output line')
        kvno, enctype, act_time, active = m.groups()
        exp_kvno, exp_enctype, exp_act_time_present, exp_active = ex
        if kvno != str(exp_kvno):
            fail('Unexpected master key version')
        if enctype != exp_enctype:
            fail('Unexpected master key enctype')
        if act_time.startswith('Active on: ') != exp_act_time_present:
            fail('Unexpected presence or absence of mkey activation time')
        if (active == ' *') != exp_active:
            fail('Master key unexpectedly active or inactive')


# Get the K/M principal.  Verify that it has the expected mkvno.  Each
# remaining argment must be a sequence of two elements: an expected
# key version and an expected enctype.
keyline_re = re.compile(r'^Key: vno (\d+), (\S+)$')
def check_master_dbent(expected_mkvno, *expected_keys):
    outlines = realm.run_kadminl('getprinc K/M').splitlines()
    mkeyline = [l for l in outlines if l.startswith('MKey: vno ')]
    if len(mkeyline) != 1 or mkeyline[0] != ('MKey: vno %d' % expected_mkvno):
        fail('Unexpected mkvno in K/M DB entry')
    keylines = [l for l in outlines if l.startswith('Key: vno ')]
    if len(keylines) != len(expected_keys):
        fail('Unexpected number of key lines in K/M DB entry')
    for line, ex in zip(keylines, expected_keys):
        m = keyline_re.match(line)
        if not m:
            fail('Unrecognized key line in K/M DB entry')
        kvno, enctype = m.groups()
        exp_kvno, exp_enctype = ex
        if kvno != str(exp_kvno):
            fail('Unexpected key version in K/M DB entry')
        if enctype != exp_enctype:
            fail('Unexpected enctype in K/M DB entry')


# Check the stash file.  Each argument must be a sequence of two
# elements: an expected key version and an expected enctype.
klist_re = re.compile(r'^\s*(\d+) K/M@KRBTEST.COM \((\S+)\)')
def check_stash(*expected):
    # Split the output of klist -e -k into lines and ignore the first three.
    outlines = realm.run([klist, '-e', '-k', stash_file]).splitlines()[3:]
    if len(outlines) != len(expected):
        fail('Unexpected number of lines in stash file klist')
    for line, ex in zip(outlines, expected):
        m = klist_re.match(line)
        if not m:
            fail('Unrecognized stash file klist line')
        kvno, enctype = m.groups()
        exp_kvno, exp_enctype = ex
        if kvno != str(exp_kvno):
            fail('Unexpected stash file klist kvno')
        if enctype != exp_enctype:
            fail('Unexpected stash file klist enctype')


# Verify that the user principal has the expected mkvno.
def check_mkvno(princ, expected_mkvno):
    out = realm.run_kadminl('getprinc ' + princ)
    if ('MKey: vno %d\n' % expected_mkvno) not in out:
        fail('Unexpected mkvno in user DB entry')


# Change the password using either kadmin.local or kadmin, then check
# the mkvno of the principal against expected_mkvno and verify that
# the running KDC can access the new key.
def change_password_check_mkvno(local, princ, password, expected_mkvno):
    cmd = 'cpw -pw %s %s' % (password, princ)
    out = local and realm.run_kadminl(cmd) or realm.run_kadmin(cmd)
    if 'changed.' not in out:
        fail('Failed to change password')
    check_mkvno(princ, expected_mkvno)
    realm.kinit(princ, password)


# Add a master key with the specified options and a random password.
def add_mkey(options):
    pw = ''.join(random.choice(string.ascii_uppercase) for x in range(5))
    realm.run([kdb5_util, 'add_mkey'] + options, input=(pw + '\n' + pw + '\n'))


# Run kdb5_util update_princ_encryption (with the dry-run option if
# specified) and verify the output against the expected mkvno, number
# of updated principals, and number of already-current principals.
mkvno_re = {False: re.compile(r'^Principals whose keys are being re-encrypted '
                              'to master key vno (\d+) if necessary:$'),
            True: re.compile(r'^Principals whose keys WOULD BE re-encrypted '
                             'to master key vno (\d+):$')}
count_re = {False: re.compile(r'^(\d+) principals processed: (\d+) updated, '
                              '(\d+) already current$'),
            True: re.compile(r'^(\d+) principals processed: (\d+) would be '
                             'updated, (\d+) already current$')}
def update_princ_encryption(dry_run, expected_mkvno, expected_updated,
                            expected_current):
    opts = ['-f', '-v']
    if dry_run:
        opts += ['-n']
    out = realm.run([kdb5_util, 'update_princ_encryption'] + opts)
    lines = out.splitlines()
    # Parse the first line to get the target mkvno.
    m = mkvno_re[dry_run].match(lines[0])
    if not m:
        fail('Unexpected first line of update_princ_encryption output')
    if m.group(1) != str(expected_mkvno):
        fail('Unexpected master key version in update_princ_encryption output')
    # Parse the last line to get the principal counts.
    m = count_re[dry_run].match(lines[-1])
    if not m:
        fail('Unexpected last line of update_princ_encryption output')
    total, updated, current = m.groups()
    if (total != str(expected_updated + expected_current) or
        updated != str(expected_updated) or current != str(expected_current)):
        fail('Unexpected counts from update_princ_encryption')


# Check the initial state of the realm.
check_mkey_list((1, defetype, True, True))
check_master_dbent(1, (1, defetype))
check_stash((1, defetype))
check_mkvno(realm.user_princ, 1)

# Check that stash will fail if a temp stash file is already present.
collisionfile = os.path.join(realm.testdir, 'stash_tmp')
f = open(collisionfile, 'w')
f.close()
output = realm.run([kdb5_util, 'stash'], expected_code=1)
if 'Temporary stash file already exists' not in output:
    fail('Did not detect temp stash file collision')
os.unlink(collisionfile)

# Add a new master key with no options.  Verify that:
# 1. The new key appears in list_mkeys but has no activation time and
#    is not active.
# 2. The new key appears in the K/M DB entry and is the current key to
#    encrypt that entry.
# 3. The stash file is not modified (since we did not pass -s).
# 4. The old key is used for password changes.
add_mkey([])
check_mkey_list((2, defetype, False, False), (1, defetype, True, True))
check_master_dbent(2, (2, defetype), (1, defetype))
change_password_check_mkvno(True, realm.user_princ, 'abcd', 1)
change_password_check_mkvno(False, realm.user_princ, 'user', 1)

# Verify that use_mkey won't make all master keys inactive.
out = realm.run([kdb5_util, 'use_mkey', '1', 'now+1day'], expected_code=1)
if 'there must be one master key currently active' not in out:
    fail('Unexpected error from use_mkey making all mkeys inactive')
check_mkey_list((2, defetype, False, False), (1, defetype, True, True))

# Make the new master key active.  Verify that:
# 1. The new key has an activation time in list_mkeys and is active.
# 2. The new key is used for password changes.
# 3. The running KDC can access the new key.
realm.run([kdb5_util, 'use_mkey', '2', 'now-1day'])
check_mkey_list((2, defetype, True, True), (1, defetype, True, False))
change_password_check_mkvno(True, realm.user_princ, 'abcd', 2)
change_password_check_mkvno(False, realm.user_princ, 'user', 2)

# Check purge_mkeys behavior with both master keys still in use.
out = realm.run([kdb5_util, 'purge_mkeys', '-f', '-v'])
if 'All keys in use, nothing purged.' not in out:
    fail('Unexpected output from purge_mkeys with both mkeys in use')

# Do an update_princ_encryption dry run and for real.  Verify that:
# 1. The target master key is 2 (the active mkvno).
# 2. nprincs - 2 principals were updated and one principal was
#    skipped (K/M is not included in the output and user was updated
#    above).
# 3. The dry run doesn't change user/admin's mkvno but the real update
#    does.
# 4. The old stashed master key is sufficient to access the DB (via
#    MKEY_AUX tl-data which keeps the current master key encrypted in
#    each of the old master keys).
update_princ_encryption(True, 2, nprincs - 2, 1)
check_mkvno(realm.admin_princ, 1)
update_princ_encryption(False, 2, nprincs - 2, 1)
check_mkvno(realm.admin_princ, 2)
realm.stop_kdc()
realm.start_kdc()
realm.kinit(realm.user_princ, 'user')

# Update all principals back to mkvno 1 and to mkvno 2 again, to
# verify that update_princ_encryption targets the active master key.
realm.run([kdb5_util, 'use_mkey', '2', 'now+1day'])
update_princ_encryption(False, 1, nprincs - 1, 0)
check_mkvno(realm.user_princ, 1)
realm.run([kdb5_util, 'use_mkey', '2', 'now-1day'])
update_princ_encryption(False, 2, nprincs - 1, 0)
check_mkvno(realm.user_princ, 2)

# Test the safety check for purging with an outdated stash file.
out = realm.run([kdb5_util, 'purge_mkeys', '-f'], expected_code=1)
if 'stash file needs updating' not in out:
    fail('Unexpected error from purge_mkeys safety check')

# Update the master stash file and check it.  Save a copy of the old
# one for a later test.
shutil.copy(stash_file, stash_file + '.old')
realm.run([kdb5_util, 'stash'])
check_stash((2, defetype), (1, defetype))

# Do a purge_mkeys dry run and for real.  Verify that:
# 1. Master key 1 is purged.
# 2. The dry run doesn't remove mkvno 1 but the real one does.
# 3. The old stash file is no longer sufficient to access the DB.
# 4. If the stash file is updated, it no longer contains mkvno 1.
# 5. use_mkey now gives an error if we refer to mkvno 1.
# 6. A second purge_mkeys gives the right message.
out = realm.run([kdb5_util, 'purge_mkeys', '-v', '-n', '-f'])
if 'KVNO: 1' not in out or '1 key(s) would be purged' not in out:
    fail('Unexpected output from purge_mkeys dry-run')
check_mkey_list((2, defetype, True, True), (1, defetype, True, False))
check_master_dbent(2, (2, defetype), (1, defetype))
out = realm.run([kdb5_util, 'purge_mkeys', '-v', '-f'])
check_mkey_list((2, defetype, True, True))
check_master_dbent(2, (2, defetype))
os.rename(stash_file, stash_file + '.save')
os.rename(stash_file + '.old', stash_file)
out = realm.run([kadmin_local, '-q', 'getprinc user'], expected_code=1)
if 'Unable to decrypt latest master key' not in out:
    fail('Unexpected error from kadmin.local with old stash file')
os.rename(stash_file + '.save', stash_file)
realm.run([kdb5_util, 'stash'])
check_stash((2, defetype))
out = realm.run([kdb5_util, 'use_mkey', '1'], expected_code=1)
if '1 is an invalid KVNO value' not in out:
    fail('Unexpected error from use_mkey with invalid kvno')
out = realm.run([kdb5_util, 'purge_mkeys', '-f', '-v'])
if 'There is only one master key which can not be purged.' not in out:
    fail('Unexpected output from purge_mkeys with one mkey')

# Add a third master key with a specified enctype.  Verify that:
# 1. The new master key receives the correct number.
# 2. The enctype argument is respected.
# 3. The new master key is stashed (by itself, at the moment).
# 4. We can roll over to the new master key and use it.
add_mkey(['-s', '-e', aes128])
check_mkey_list((3, aes128, False, False), (2, defetype, True, True))
check_master_dbent(3, (3, aes128), (2, defetype))
check_stash((3, aes128))
realm.run([kdb5_util, 'use_mkey', '3', 'now-1day'])
update_princ_encryption(False, 3, nprincs - 1, 0)
check_mkey_list((3, aes128, True, True), (2, defetype, True, False))
check_mkvno(realm.user_princ, 3)

# Regression test for #7994 (randkey does not update principal mkvno)
# and #7995 (-keepold does not re-encrypt old keys).
add_mkey(['-s'])
realm.run([kdb5_util, 'use_mkey', '4', 'now-1day'])
realm.run_kadminl('cpw -randkey -keepold %s' % realm.user_princ)
# With #7994 unfixed, mkvno of user will still be 3.
check_mkvno(realm.user_princ, 4)
# With #7995 unfixed, old keys are still encrypted with mkvno 3.
update_princ_encryption(False, 4, nprincs - 2, 1)
realm.run([kdb5_util, 'purge_mkeys', '-f'])
out = realm.run_kadminl('xst -norandkey %s' % realm.user_princ)
if 'Decrypt integrity check failed' in out or 'added to keytab' not in out:
    fail('Preserved old key data not updated to new master key')

realm.stop()

# Load a dump file created with krb5 1.6, before the master key
# rollover changes were introduced.  Write out an old-format stash
# file consistent with the dump's master password ("footes").  The K/M
# entry in this database will not have actkvno tl-data because it was
# created prior to master key rollover support.  Verify that:
# 1. We can access the database using the old-format stash file.
# 2. list_mkeys displays the same list as for a post-1.7 KDB.
dumpfile = os.path.join(srctop, 'tests', 'dumpfiles', 'dump.16')
os.remove(stash_file)
f = open(stash_file, 'w')
f.write(struct.pack('=HL24s', 16, 24,
                    '\xF8\x3E\xFB\xBA\x6D\x80\xD9\x54\xE5\x5D\xF2\xE0'
                    '\x94\xAD\x6D\x86\xB5\x16\x37\xEC\x7C\x8A\xBC\x86'))
f.close()
realm.run([kdb5_util, 'load', dumpfile])
nprincs = len(realm.run_kadminl('listprincs').splitlines()) - 1
check_mkvno('K/M', 1)
check_mkey_list((1, des3, True, True))

# Create a new master key and verify that, without actkvkno tl-data:
# 1. list_mkeys displays the same as for a post-1.7 KDB.
# 2. update_princ_encryption still targets mkvno 1.
# 3. libkadm5 still uses mkvno 1 for key changes.
# 4. use_mkey creates the same list as for a post-1.7 KDB.
add_mkey([])
check_mkey_list((2, defetype, False, False), (1, des3, True, True))
update_princ_encryption(False, 1, 0, nprincs - 1)
realm.run_kadminl('addprinc -randkey ' + realm.user_princ)
check_mkvno(realm.user_princ, 1)
realm.run([kdb5_util, 'use_mkey', '2', 'now-1day'])
check_mkey_list((2, defetype, True, True), (1, des3, True, False))

success('Master key rollover tests')
