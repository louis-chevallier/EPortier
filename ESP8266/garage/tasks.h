#include "util.h"

#include <DoubleLinkedList.hpp>
namespace tasks {
  ESP8266Timer ITimer;
  typedef void (*task)  ();
  struct Task {
    long date_ms; // when the task is scheduled
    unsigned int period_ms; // repeat if > 0
    task t; // callback
    int id;
    Task(task _t = NULL, long when_ms=0, bool repeat=0, long _period_ms=0, int _id=0) :
      date_ms(when_ms), period_ms(_period_ms), t(_t), id(_id){} 
  };

  typedef MicroTuple<int, task> IF;

  const long SEC_MS = 1000;
  
  // list ordonnée
  DoubleLinkedList<Task> tasks;

  volatile uint32_t lastMillis = 0;


  typedef void (*timer_callback)  ();

  unsigned int index(long w) {
    /* assumes tasks sorted by increasing date_mc
     * yield first i st tasks[i].date_mc > w
     */
    unsigned int i(0);
    for (i = 0; i < tasks.getSize() && tasks.get(i).date_ms <= w; i++) {}
    return i;
  }

  void restart();
  String dump() {
    String ss;
    for (int i = 0; i < tasks.getSize(); i++) {
      ss += "(";
      ss += tasks.get(i).date_ms;
      ss += ",";
      ss += tasks.get(i).id;
      ss += ") ";
    }
    return ss;
  }  

  void IRAM_ATTR TimerHandler()
  {
    long now = millis();
    EKOX(now);
    EKOX(dump());    
    if (tasks.getSize() > 0) {

      Task t(tasks.get(0));
      if (t.date_ms < now ) {
        EKOX(t.date_ms);
        EKOX(now);
        //EKOT("executing task with date passed !");
      }
      
      t.t();
      auto p = t.period_ms;
      tasks.remove(0);
      EKO();
      if (p > 0) {
        EKO();
        long w = now + p;
        EKOX(w);
        Task nt(t.t, w, p > 0, p, t.id);
        auto ind = index(w);
        EKOX(ind);
        EKOX(dump());           
        tasks.addAtIndex(ind, t);
        EKOX(dump());   
        //assert(index(w) == ind);
        EKO();
      }
      if (tasks.getSize() > 0) {
        restart();
      }
      EKO();
    } else {
      ITimer.detachInterrupt();
    }
  }
  
  void restart() {
    auto now = millis();    
    auto dd = tasks.get(0).date_ms;
    ITimer.detachInterrupt();
    long delay = dd-now;
    if (delay < 0) {
      EKOT(" first task date passed !");
      delay = 0;
    }
    ITimer.attachInterruptInterval(delay * 1000, TimerHandler);
  }  
  int num(0);

  struct Args {
    unsigned int delay_ms = 0;
    task t;
    bool repeat=1>2;
    unsigned int period = 0;
  };
  

  int apres(unsigned int delay_ms, task t, bool repeat=1>2, unsigned int period = 0 ) {
    long now = millis();
    EKOX(now);
    long when = now + delay_ms;
    EKOX(delay_ms);
    Task nt(t, when, repeat, period, num);
    int ind = index(when);
    EKOX(ind);
    tasks.addAtIndex(ind, nt);
    EKOX(index(nt.date_ms));
    //assert(index(nt.date_mc) == ind);    
    restart();
    num ++;
    EKOX(dump());
    return num;
  }

  int apres(Args args) {
    return apres(args.delay_ms, args.t, args.repeat, args.period);
  }


  void test() {
    tasks::apres(2 * SEC_MS, [](){
      EKO();
      EKOX(dump());
    });
    /*
    tasks::apres(SEC_MS, [](){
      EKO();
      EKOX(dump());      
    }, true, 4*SEC_MS);
    */
    tasks::apres({
        .delay_ms = SEC_MS,
        .t= [](){
          EKO();
          EKOX(dump());      
        },
        .repeat = true,
        .period = 4*SEC_MS});


    
    tasks::apres(3 * SEC_MS, [](){
      EKO();
      EKOX(dump());      
    });
    EKO();
  }    
  
  void test1() {
    auto t = [](){
      1;
    };
    {
      long ddd = 12;
      Task nt(t, ddd, 0,0,0);
      int ind = index(ddd);
      EKOX(ind);
      tasks.addAtIndex(ind, nt);
      EKOX(index(ddd));
      EKOX(dump());
    }

    {
      long ddd = 13;
      Task nt(t, ddd, 0,0,1);
      int ind = index(ddd);
      EKOX(ind);
      tasks.addAtIndex(ind, nt);
      EKOX(index(ddd));
      EKOX(dump());
    }

    {
      long ddd = 22;
      Task nt(t, ddd, 0,0,2);
      int ind = index(ddd);
      EKOX(ind);
      tasks.addAtIndex(ind, nt);
      EKOX(index(ddd));
      EKOX(dump());
    }

    {
      long ddd = 3;
      Task nt(t, ddd, 0,0,3);
      int ind = index(ddd);
      EKOX(ind);
      tasks.addAtIndex(ind, nt);
      EKOX(index(ddd));
      EKOX(dump());
    }
    {
      long ddd = 14;
      Task nt(t, ddd, 0,0,33);
      int ind = index(ddd);
      EKOX(ind);
      tasks.addAtIndex(ind, nt);
      EKOX(index(ddd));
      EKOX(dump());
    }
    {
      long ddd = 13;
      Task nt(t, ddd, 0,0,4);
      int ind = index(ddd);
      EKOX(ind);
      tasks.addAtIndex(ind, nt);
      EKOX(index(ddd));
      EKOX(dump());
    }    

  }

  
}
