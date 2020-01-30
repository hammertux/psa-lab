Tests for assigment 2
=====================

0_check_if_allocate_on_write.trf
--------------------------------
('P0', 'WRITE', 256)
('P1', 'WRITE', 512)

('P0', 'NOP', 0)
('P1', 'NOP', 0)

('P0', 'NOP', 0)
('P1', 'NOP', 0)

('P0', 'READ', 256)
('P1', 'READ', 512)

Expect:
- allocate on write: write misses and read hits. ~200 cycles. (if
  blocking write). read hits: 2


0_check_if_allocate_on_write_with_nops.trf
------------------------------------------
('P0', 'WRITE', 256)
('P1', 'WRITE', 512)

('P0', 'NOP', 0)
('P1', 'NOP', 0)

('P0', 'NOP', 0)
('P1', 'NOP', 0)

... // 200 nops

('P0', 'READ', 256)
('P1', 'READ', 512)

Expect:
Same as above but with extra nops: write misses and read hits
Allocate on write. 200 cycles + 200 nops + 1 cycle hits. ~ 400 cycles


1_read_write_invalidate.trf
---------------------------
('P0', 'READ', 32)
('P1', 'NOP', 0)

('P0', 'NOP', 0)
('P1', 'READ', 512)

('P0', 'NOP', 0)
('P1', 'NOP', 0)

('P0', 'NOP', 0)
('P1', 'WRITE', 32)

('P0', 'READ', 768)
('P1', 'NOP', 0)

('P0', 'READ', 32)
('P1', 'NOP', 0)

Expect:
last P0 read to miss.  So no cache hits at all.
Expected delay: ~300 (alloc on write, blocking writes)


2_read_write_overlap.trf
------------------------
('P0', 'READ', 32)
('P1', 'NOP', 0)

('P0', 'NOP', 0)
('P1', 'WRITE', 32)

('P0', 'NOP', 0)
('P1', 'NOP', 0)

('P0', 'READ', 32)
('P1', 'NOP', 0)

Expect:
Second P0 read to miss because block was invalidated by P1.
Unless write alloc is blocking and test 1 passes and read is not a
read-exclusive. This indicates invalidates comes after read.
delay: ~200 (longer on allocate)


Test for assignment 3
=====================

3_M-O.trf
---------
('P0', 'WRITE', 32)
('P1', 'WRITE', 512)

('P0', 'NOP', 0)
('P1', 'NOP', 0)

('P0', 'NOP', 0)
('P1', 'NOP', 0)

('P0', 'NOP', 0)
('P1', 'READ', 32)

Expect:
C0 should go from M to O, C1 should go from I to S.
No hits on the cache.
The "data" for C1's read comes from C0 via a cache-to-cache
transfer. 
Expected memory accesses: 2 reads, no writes
Delay: ~100+

