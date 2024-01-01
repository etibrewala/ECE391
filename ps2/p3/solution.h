// // #ifndef SOLUTION_H
// // #define SOLUTION_H
// // #include "spinlock_ece391.h"


// // typedef struct ps_enter_exit_lock {
// //   // Fill this out!!!
// //   volatile unsigned int professor_line;
// //   volatile unsigned int ta_line;
// //   volatile unsigned int student_line;

// //   volatile unsigned int professor_lab;
// //   volatile unsigned int ta_lab;
// //   volatile unsigned int student_lab;

// //   volatile unsigned int ppl_in_room;

// //   spinlock_t *lock;
// // } ps_lock;

// // ps_lock ps_lock_create(spinlock_t *lock);
// // void professor_enter(ps_lock *ps);
// // void professor_exit(ps_lock *ps);
// // void ta_enter(ps_lock *ps);
// // void ta_exit(ps_lock *ps);
// // void student_enter(ps_lock *ps);
// // void student_exit(ps_lock *ps);

// // #endif /* SOLUTION_H */
// #ifndef SOLUTION_H
// #define SOLUTION_H
// #include "spinlock_ece391.h"


// typedef struct ps_enter_exit_lock {
//   int nprof_line;
//   int nta_line;
//   int nstu_line;

//   int nprof_room;
//   int nta_room;
//   int nstu_room;
  
//   int nroom;
//   spinlock_t lock;
// } ps_lock;

// ps_lock ps_lock_create(spinlock_t *lock);
// void professor_enter(ps_lock *ps);
// void professor_exit(ps_lock *ps);
// void ta_enter(ps_lock *ps);
// void ta_exit(ps_lock *ps);
// void student_enter(ps_lock *ps);
// void student_exit(ps_lock *ps);

// #endif /* SOLUTION_H */
#ifndef SOLUTION_H
#define SOLUTION_H
#include "spinlock_ece391.h"


// typedef struct ps_enter_exit_lock {
//   // Fill this out!!!
//   int student_ct;
//   int ta_ct;
//   int prof_ct;
  
//   int prof_line;
//   int ta_line;
//   int student_line;

//   int total_ct;
  
//   spinlock_t thespinlock;

//   int student_ct;
//   int ta_ct;
//   int prof_ct;
//   int prof_line;
//   int ta_line;
//   int student_line;
//   spinlock_t *thespinlock;
// } ps_lock;

typedef struct ps_enter_exit_lock {
  // Fill this out!!!
  spinlock_t lock;
  int students_inside;
  int ta_inside;
  int prof_inside;
  int students_waiting;
  int ta_waiting;
  int prof_waiting;
  int total;
} ps_lock;

ps_lock ps_lock_create(spinlock_t *lock);
void professor_enter(ps_lock *ps);
void professor_exit(ps_lock *ps);
void ta_enter(ps_lock *ps);
void ta_exit(ps_lock *ps);
void student_enter(ps_lock *ps);
void student_exit(ps_lock *ps);

#endif /* SOLUTION_H */
