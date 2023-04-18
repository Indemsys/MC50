#ifndef DEMO_THREAD_H
  #define DEMO_THREAD_H


  #define HABROMETER_TASK_PRIO    12

typedef struct
{
    float  karma                ;
    float  rating               ;
    float  number_of_subscribers;
    float  number_of_votes      ;
    float  number_of_views      ;
    float  number_of_bookmarks  ;
}
T_hamrometer_results;

extern T_hamrometer_results  habr_results;

uint32_t Thread_Habr_Karma_create(void);


#endif // DEMO_THREAD_H



