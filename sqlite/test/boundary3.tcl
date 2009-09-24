puts {# 2008 December 11
#
# The author disclaims copyright to this source code.  In place of
# a legal notice, here is a blessing:
#
#    May you do good and not evil.
#    May you find forgiveness for yourself and forgive others.
#    May you share freely, never taking more than you give.
#
#***********************************************************************
# This file implements regression tests for SQLite library.
#
# This file is automatically generated from a separate TCL script.
# This file seeks to exercise integer boundary values.
#
# $Id: boundary3.tcl,v 1.3 2009/01/02 15:45:48 shane Exp $

set testdir [file dirname $argv0]
source $testdir/tester.tcl

# Many of the boundary tests depend on a working 64-bit implementation.
if {![working_64bit_int]} { finish_test; return }
}

expr srand(0)

# Generate interesting boundary numbers
#
foreach x {
  0
  1
  0x7f
  0x7fff
  0x7fffff
  0x7fffffff
  0x7fffffffff
  0x7fffffffffff
  0x7fffffffffffff
  0x7fffffffffffffff
} {
  set x [expr {wide($x)}]
  set boundarynum($x) 1
  set boundarynum([expr {$x+1}]) 1
  set boundarynum([expr {-($x+1)}]) 1
  set boundarynum([expr {-($x+2)}]) 1
  set boundarynum([expr {$x+$x+1}]) 1
  set boundarynum([expr {$x+$x+2}]) 1
}
set x [expr {wide(127)}]
for {set i 1} {$i<=9} {incr i} {
  set boundarynum($x) 1
  set boundarynum([expr {$x+1}]) 1
  set x [expr {wide($x*128 + 127)}]
}

# Scramble the $inlist into a random order.
#
proc scramble {inlist} {
  set y {}
  foreach x $inlist {
    lappend y [list [expr {rand()}] $x]
  }
  set y [lsort $y]
  set outlist {}
  foreach x $y {
    lappend outlist [lindex $x 1]
  }
  return $outlist
}

# A simple selection sort.  Not trying to be efficient.
#
proc sort {inlist} {
  set outlist {}
  set mn [lindex $inlist 0]
  foreach x $inlist {
    if {$x<$mn} {set mn $x}
  }
  set outlist $mn
  set mx $mn
  while {1} {
    set valid 0
    foreach x $inlist {
      if {$x>$mx && (!$valid || $mn>$x)} {
        set mn $x
        set valid 1
      }
    }
    if {!$valid} break
    lappend outlist $mn
    set mx $mn
  }
  return $outlist
}

# Reverse the order of a list
#
proc reverse {inlist} {
  set i [llength $inlist]
  set outlist {}
  for {incr i -1} {$i>=0} {incr i -1} {
    lappend outlist [lindex $inlist $i]
  }
  return $outlist
}

set nums1 [scramble [array names boundarynum]]
set nums2 [scramble [array names boundarynum]]

set tname boundary3
puts "do_test $tname-1.1 \173"
puts "  db eval \173"
puts "    CREATE TABLE t1(a,x);"
set a 0
foreach r $nums1 {
  incr a
  set t1ra($r) $a
  set t1ar($a) $r
  set x [format %08x%08x [expr {wide($r)>>32}] $r]
  set t1rx($r) $x
  set t1xr($x) $r
  puts "    INSERT INTO t1(oid,a,x) VALUES($r,$a,'$x');"
}
puts "    CREATE INDEX t1i1 ON t1(a);"
puts "    CREATE INDEX t1i2 ON t1(x);"
puts "  \175"
puts "\175 {}"

puts "do_test $tname-1.2 \173"
puts "  db eval \173"
puts "    SELECT count(*) FROM t1"
puts "  \175"
puts "\175 {64}"

puts "do_test $tname-1.3 \173"
puts "  db eval \173"
puts "    CREATE TABLE t2(r,a);"
puts "    INSERT INTO t2 SELECT rowid, a FROM t1;"
puts "    CREATE INDEX t2i1 ON t2(r);"
puts "    CREATE INDEX t2i2 ON t2(a);"
puts "    INSERT INTO t2 VALUES(9.22337303685477580800e+18,65);"
set t1ra(9.22337303685477580800e+18) 65
set t1ar(65) 9.22337303685477580800e+18)
puts "    INSERT INTO t2 VALUES(-9.22337303685477580800e+18,66);"
set t1ra(-9.22337303685477580800e+18) 66
set t1ar(66) -9.22337303685477580800e+18)
puts "    SELECT count(*) FROM t2;"
puts "  \175"
puts "\175 {66}"

set nums3 $nums2
lappend nums3 9.22337303685477580800e+18
lappend nums3 -9.22337303685477580800e+18

set i 0
foreach r $nums3 {
  incr i

  set r5 $r.5
  set r0 $r.0
  if {abs($r)<9.22337203685477580800e+18} {
    set x $t1rx($r)
    set a $t1ra($r)
    puts "do_test $tname-2.$i.1 \173"
    puts "  db eval \173"
    puts "    SELECT t1.* FROM t1, t2 WHERE t1.rowid=$r AND t2.a=t1.a"
    puts "  \175"
    puts "\175 {$a $x}"
    puts "do_test $tname-2.$i.2 \173"
    puts "  db eval \173"
    puts "    SELECT t2.* FROM t1 JOIN t2 USING(a) WHERE x='$x'"
    puts "  \175"
    puts "\175 {$r $a}"
    puts "do_test $tname-2.$i.3 \173"
    puts "  db eval \173"
    puts "    SELECT t1.rowid, x FROM t1 JOIN t2 ON t2.r=t1.rowid WHERE t2.a=$a"
    puts "  \175"
    puts "\175 {$r $x}"
  }

  foreach op {> >= < <=} subno {gt ge lt le} {

    ################################################################ 2.x.y.1
    set rset {}
    set aset {}
    foreach rx $nums2 {
      if "\$rx $op \$r" {
        lappend rset $rx
        lappend aset $t1ra($rx)
      }
    }
    puts "do_test $tname-2.$i.$subno.1 \173"
    puts "  db eval \173"
    puts "    SELECT t2.a FROM t1 JOIN t2 USING(a)"
    puts "     WHERE t1.rowid $op $r ORDER BY t2.a"
    puts "  \175"
    puts "\175 {[sort $aset]}"

    ################################################################ 2.x.y.2
    puts "do_test $tname-2.$i.$subno.2 \173"
    puts "  db eval \173"
    puts "    SELECT t2.a FROM t2 NATURAL JOIN t1"
    puts "     WHERE t1.rowid $op $r ORDER BY t1.a DESC"
    puts "  \175"
    puts "\175 {[reverse [sort $aset]]}"


    ################################################################ 2.x.y.3
    set ax $t1ra($r)
    set aset {}
    foreach rx [sort $rset] {
      lappend aset $t1ra($rx)
    }
    puts "do_test $tname-2.$i.$subno.3 \173"
    puts "  db eval \173"
    puts "    SELECT t1.a FROM t1 JOIN t2 ON t1.rowid $op t2.r"
    puts "     WHERE t2.a=$ax"
    puts "     ORDER BY t1.rowid"
    puts "  \175"
    puts "\175 {$aset}"

    ################################################################ 2.x.y.4
    set aset {}
    foreach rx [reverse [sort $rset]] {
      lappend aset $t1ra($rx)
    }
    puts "do_test $tname-2.$i.$subno.4 \173"
    puts "  db eval \173"
    puts "    SELECT t1.a FROM t1 JOIN t2 ON t1.rowid $op t2.r"
    puts "     WHERE t2.a=$ax"
    puts "     ORDER BY t1.rowid DESC"
    puts "  \175"
    puts "\175 {$aset}"

    ################################################################ 2.x.y.5
    set aset {}
    set xset {}
    foreach rx $rset {
      lappend xset $t1rx($rx)
    }
    foreach x [sort $xset] {
      set rx $t1xr($x)
      lappend aset $t1ra($rx)
    }
    puts "do_test $tname-2.$i.$subno.5 \173"
    puts "  db eval \173"
    puts "    SELECT t1.a FROM t1 JOIN t2 ON t1.rowid $op t2.r"
    puts "     WHERE t2.a=$ax"
    puts "     ORDER BY x"
    puts "  \175"
    puts "\175 {$aset}"

    ################################################################ 2.x.y.10
    if {[string length $r5]>15} continue
    set rset {}
    set aset {}
    foreach rx $nums2 {
      if "\$rx $op \$r0" {
        lappend rset $rx
      }
    }
    foreach rx [sort $rset] {
      lappend aset $t1ra($rx)
    }
    puts "do_test $tname-2.$i.$subno.10 \173"
    puts "  db eval \173"
    puts "    SELECT t1.a FROM t1 JOIN t2 ON t1.rowid $op CAST(t2.r AS real)"
    puts "     WHERE t2.a=$ax"
    puts "     ORDER BY t1.rowid"
    puts "  \175"
    puts "\175 {$aset}"

    ################################################################ 2.x.y.11
    set aset {}
    foreach rx [reverse [sort $rset]] {
      lappend aset $t1ra($rx)
    }
    puts "do_test $tname-2.$i.$subno.11 \173"
    puts "  db eval \173"
    puts "    SELECT t1.a FROM t1 JOIN t2 ON t1.rowid $op CAST(t2.r AS real)"
    puts "     WHERE t2.a=$ax"
    puts "     ORDER BY t1.rowid DESC"
    puts "  \175"
    puts "\175 {$aset}"
  }

}


puts {finish_test}
