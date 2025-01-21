#include "util.h"

#include <DoubleLinkedList.hpp>

typedef unsigned long long ULL;

namespace tasks {
  ESP8266Timer ITimer;
  typedef void (*task)  ();
  struct Task {
    unsigned long long date_mc; // when the task is scheduled : == date wrt to millis      
    unsigned long period_mc; // repeat if > 0
    task t; // callback
    int id;
    Task(task _t = NULL, long _date_mc=0, long _period_mc=0, int _id=0) :
      date_mc(_date_mc), period_mc(_period_mc), t(_t), id(_id){
      //EKOX(id);
    } 
  };

  typedef MicroTuple<int, task> IF;

  const long SEC_MC = 1000000;
  
  // list ordonnée
  DoubleLinkedList<Task> tasks;

  volatile uint32_t lastMillis = 0;

  typedef void (*timer_callback)  ();

  unsigned int index(ULL date_mc) {
    /* smaller i st tasks_i.date_mc >  date_mc
     */
    int i;
    for (i = 0; i < tasks.getSize() && tasks.get(i).date_mc <= date_mc; i++) {
    }
    return i;
  }

  void restart();

  
  String dump() {
    String ss;
    for (int i = 0; i < tasks.getSize(); i++) {
      ss += "(";
      ss += tasks.get(i).date_mc;
      ss += ",";
      ss += tasks.get(i).id;
      ss += ") ";
    }
    return ss;
  }  

  void check() {
    if (tasks.getSize() > 0) {
      for (int i = 1; i < tasks.getSize(); i++) {
        assert(tasks[i].date_mc >= tasks[i-1].date_mc);
      }
    }
  }  

  void IRAM_ATTR TimerHandler()
  {
    ULL now = millis();
    //EKOX(now);
    //EKOX(dump());    
    if (tasks.getSize() > 0) {
      //EKO();
      const Task &t(tasks.get(0));
      long long ll = t.date_mc;
      long long left = ll - now*1000;
      //EKOX(ll);
      //EKOX(now*1000);
      //EKOX(left);
      if (left < 1000)  {
        tasks.remove(0);
        int ___________________executing = t.id;
        //EKOX(___________________executing);
        t.t();
        auto p = t.period_mc;
        //EKO();
        if (p > 0) {
          //EKO();
          ULL w_mc = now*1000 + p;
          //EKOX(w_mc);
          //EKOX(dump());                   
          auto ind = index(w_mc);
          //EKOX(ind);
          Task nt(t.t, w_mc, p, t.id);
          //EKOX(dump());           
          tasks.addAtIndex(ind, nt);
          //EKOX(dump());
          //check();
          //assert(index(w) == ind);
          //EKO();
        }
        //EKO();
        if (tasks.getSize() > 0) {
          restart();
        }
        //EKO();
      } else {
        //EKO();
        restart();
      }
      //EKO();
    } else {
      //EKO();
      ITimer.detachInterrupt();
    }
  }
  
  void restart() {
    //EKO();
    ITimer.detachInterrupt();
    if (tasks.getSize() > 0) {
      ULL  now = millis();
      //EKO();
      auto dd = tasks.get(0).date_mc;
      long long ddd = dd - now * 1000;
      auto delay_mc = ddd > 0 ? ddd :  0;
      ITimer.attachInterruptInterval(delay_mc, TimerHandler);
      //EKO();
    }
  }  
  int num(0);

  struct Args {
    ULL date_mc = 0;
    task t;
    ULL period_mc = 0;
  };
  

  int apres(ULL delay_mc, task t, ULL period_mc = 0 ) {
    ULL  now = millis();
    //EKO();
    auto date_mc = delay_mc + now * 1000;
    //EKO();
    Task nt(t, date_mc, period_mc, num);
    int ind = index(date_mc);
    //EKOX(ind);
    tasks.addAtIndex(ind, nt);
    //EKOX(index(nt.date_mc));
    //check();
    //assert(index(nt.date_mc) == ind);    
    restart();
    num ++;
    //EKOX(dump());
    return num-1;
  }

  int apres(Args args) {
    return apres(args.date_mc, args.t, args.period_mc);
  }


  void test() {

    unsigned long long s;
    //EKOX(sizeof(s));
    
    auto t0 = tasks::apres(2 * SEC_MC, [](){
      EKO();
      //EKOX(dump());
    });
    EKOX(t0);
    /*
    tasks::apres(SEC_MC, [](){
      EKO();
      EKOX(dump());      
    }, true, 4*SEC_MC);
    */
    auto t1 = tasks::apres({
        .date_mc = SEC_MC,
        .t= [](){
          EKO();
          EKOX(dump());      
        },
        .period_mc = 4*SEC_MC});
    EKOX(t1);
    
    auto t2 = tasks::apres(3 * SEC_MC, [](){
      EKO();
      EKOX(dump());      
    });
    EKOX(t2);

    auto t3 = tasks::apres(9 * SEC_MC, [](){
      EKO();
      EKOX(dump());
      auto t4 = tasks::apres(1 * SEC_MC, [](){
        EKO();
        EKOX(dump());
      });
      EKOX(t4);
    });
    EKOX(t3);
    EKOX(dump());    
  }    
  
  void test1() {
    auto t = [](){
      1;
    };
    {
      long int ddd = 12;
      Task nt(t, ddd, 0,0);
      int ind = index(ddd);
      EKOX(ind);
      tasks.addAtIndex(ind, nt);
      EKOX(index(ddd));
      EKOX(dump());
    }

    {
      long ddd = 13;
      Task nt(t, ddd, 0,1);
      int ind = index(ddd);
      EKOX(ind);
      tasks.addAtIndex(ind, nt);
      EKOX(index(ddd));
      EKOX(dump());
    }

    {
      long ddd = 22;
      Task nt(t, ddd, 0,2);
      int ind = index(ddd);
      EKOX(ind);
      tasks.addAtIndex(ind, nt);
      EKOX(index(ddd));
      EKOX(dump());
    }

    {
      long ddd = 3;
      Task nt(t, ddd, 0,3);
      int ind = index(ddd);
      EKOX(ind);
      tasks.addAtIndex(ind, nt);
      EKOX(index(ddd));
      EKOX(dump());
    }
    {
      long ddd = 14;
      Task nt(t, ddd, 0,33);
      int ind = index(ddd);
      EKOX(ind);
      tasks.addAtIndex(ind, nt);
      EKOX(index(ddd));
      EKOX(dump());
    }
    {
      long ddd = 13;
      Task nt(t, ddd, 0,4);
      int ind = index(ddd);
      EKOX(ind);
      tasks.addAtIndex(ind, nt);
      EKOX(index(ddd));
      EKOX(dump());
    }    

  }

  
}
