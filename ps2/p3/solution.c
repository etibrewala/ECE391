// #include "spinlock_ece391.h"
// #include "solution.h"
// #include <bits/types.h>

// #define TRUE 1
// #define FALSE 0
// #define MAX_ROOM_OCC 20

// typedef int bool;

// unsigned long flags;

// ps_lock ps_lock_create(spinlock_t *lock) {
//   ps_lock ps;
//   ps.nprof_line = 0;
//   ps.nta_line = 0;
//   ps.nstu_line = 0;

//   ps.nprof_room = 0;
//   ps.nta_room = 0;
//   ps.nstu_room = 0;
  
//   ps.nroom = 0;

//   ps.lock = *lock;
// }

// void professor_enter(ps_lock *ps) {
  
//   while (ps->nroom>=20 || ps->nta_room>0 || ps->nstu_room>0);
//   spinlock_lock_ece391(&ps->lock);
//   ps->nprof_line--;
//   ps->nprof_room++;
//   ps->nroom++;
//   spinlock_unlock_ece391(&ps->lock);
// }

// void professor_exit(ps_lock *ps) {
//   spinlock_lock_ece391(&ps->lock);
//   ps->nprof_room--;
//   ps->nroom--;
//   spinlock_unlock_ece391(&ps->lock);

// }

// void ta_enter(ps_lock *ps) {

//   while (ps->nroom>=20 || ps->nprof_room>0 || ps->nprof_line>0);
//   spinlock_lock_ece391(&ps->lock);
//   ps->nta_line--;
//   ps->nta_room++;
//   ps->nroom++;
//   spinlock_unlock_ece391(&ps->lock);
// }

// void ta_exit(ps_lock *ps) {
//   spinlock_lock_ece391(&ps->lock);
//   ps->nta_room--;
//   ps->nroom--;
//   spinlock_unlock_ece391(&ps->lock);
// }

// void student_enter(ps_lock *ps) {
//   while (ps->nroom>=20 || ps->nprof_room>0 || ps->nprof_line>0 || ps->nta_line>0);
//   spinlock_lock_ece391(&ps->lock);
//   ps->nstu_line--;
//   ps->nstu_room++;
//   ps->nroom++;
//   spinlock_unlock_ece391(&ps->lock);
// }

// void student_exit(ps_lock *ps) {
//   spinlock_lock_ece391(&ps->lock);
//   ps->nstu_room--;
//   ps->nroom--;
//   spinlock_unlock_ece391(&ps->lock);
// }


#include "spinlock_ece391.h"
#include "solution.h"
#include <bits/types.h>


ps_lock ps_lock_create(spinlock_t *lock) {
  // Fill this out!!!
  ps_lock ps;
  ps.lock = *lock;
  ps.students_inside = 0;
  ps.ta_inside = 0;
  ps.prof_inside = 0;
  ps.students_waiting = 0;
  ps.ta_waiting = 0;
  ps.prof_waiting = 0;
  ps.total = 0;
  return ps;
}

void professor_enter(ps_lock *ps) {
  // Fill this out!!!
  spinlock_lock_ece391(&ps->lock);

  // Professors must wait until there are no students or TAs inside the lab.
  while (ps->students_inside > 0 || ps->ta_inside > 0) {
      spinlock_unlock_ece391(&ps->lock);
      spinlock_lock_ece391(&ps->lock);
  }

  ps->prof_waiting--;
  ps->prof_inside++;
  ps->total++;
  // ps->students_inside = 0;
  // ps->ta_inside = 0;


  spinlock_unlock_ece391(&ps->lock);
}

void professor_exit(ps_lock *ps) {
  // Fill this out!!!
  spinlock_lock_ece391(&ps -> lock);

  ps ->prof_inside--;
  ps ->total--;

  spinlock_unlock_ece391(&ps -> lock);
}

void ta_enter(ps_lock *ps) {
  // Fill this out!!!
  spinlock_lock_ece391(&ps->lock);

  // TAs can enter only if no professors are inside.
  while (ps->prof_inside > 0 || ps->total >=20) {
      spinlock_unlock_ece391(&ps->lock);
      spinlock_lock_ece391(&ps->lock);
  }

  ps->ta_waiting--;
  ps->ta_inside++;
  ps->total++;
  // ps->prof_inside = 0;

  spinlock_unlock_ece391(&ps->lock);
}

void ta_exit(ps_lock *ps) {
  // Fill this out!!!
  spinlock_lock_ece391(&ps -> lock);

  ps ->ta_inside--;
  ps ->total--;

  spinlock_unlock_ece391(&ps -> lock);
}

void student_enter(ps_lock *ps) {
  // Fill this out!!!
  spinlock_lock_ece391(&ps->lock);

    // Students can enter only if no professors are inside and the lab is not full.
  while (ps->prof_inside > 0 || ps->total >= 20) {
        spinlock_unlock_ece391(&ps->lock);
        spinlock_lock_ece391(&ps->lock);
  }

  // Students and TAs can occupy the lab at the same time, so check for TAs inside.
  while (ps->ta_inside == 0 && ps->prof_waiting > 0) {
        spinlock_unlock_ece391(&ps->lock);
        spinlock_lock_ece391(&ps->lock);
  }

  ps->students_waiting--;
  ps->students_inside++;
  ps->total++;
  // ps->prof_inside = 0;


  spinlock_unlock_ece391(&ps->lock);
}

void student_exit(ps_lock *ps) {
  // Fill this out!!!
  spinlock_lock_ece391(&ps -> lock);

  ps ->students_inside--;
  ps ->total--;

  spinlock_unlock_ece391(&ps -> lock);
}
