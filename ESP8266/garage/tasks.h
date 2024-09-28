#include <DoubleLinkedList.hpp>
namespace tasks {
  ESP8266Timer ITimer;
  typedef void (*task)  ();
  struct Task {
    long date_mc; // when the task is scheduled
    unsigned int period_mc; // repeat if > 0
    task t; // callback
    int id;
    Task(task _t = NULL, long when_mc=0, bool repeat=0, long _period_mc=0, int _id=0) :
      date_mc(when_mc), period_mc(_period_mc), t(_t), id(_id){} 
  };

  typedef MicroTuple<int, task> IF;


  
  // list ordonnée
  DoubleLinkedList<Task> tasks;

  
#define TIMER_INTERVAL_MS        1000
  volatile uint32_t lastMillis = 0;


  typedef void (*timer_callback)  ();

  unsigned int index(long w) {
    /* assumes tasks sorted by increasing date_mc
     * yield first i st tasks[i].date_mc > w
     */
    unsigned int i(0);
    for (i = 0; i < tasks.getSize() && tasks.get(i).date_mc < w; i++) {}
    return i;
  }

  void restart();
  
  void IRAM_ATTR TimerHandler()
  {
    long now = micros();    
    Task t(tasks.get(0));
    if (t.date_mc < now ) {
      Serial.println("executing task with date passed !");
    }
    
    t.t();
    auto p = t.period_mc;
    tasks.remove(0);    
    if (p > 0) {
      long w = now + p;
      Task nt(t.t, w, p > 0, p, t.id);
      auto ind = index(w);
      tasks.addAtIndex(ind, t);

      assert(index(w) == ind);
      
    }
    if (tasks.getSize() > 0) {
      restart();
    }
  }
  
  void restart() {
    auto now = micros();    
    auto dd = tasks.get(0).date_mc;
    ITimer.detachInterrupt();
    int delay = dd-now;
    if (delay < 0) {
      Serial.println(" first task date passed !");
      delay = 0;
    }
    ITimer.attachInterruptInterval(delay, TimerHandler);
  }  
  int num(0);

  int apres(unsigned int delay, task t, bool repeat=1>2, unsigned int period = 0 ) {
    long now = micros();
    long when = now + delay;
    Task nt(t, now + delay, repeat, period, num);
    int ind = index(now+delay);
    tasks.addAtIndex(ind, nt);
    assert(index(nt.date_mc) == ind);    
    restart();
    num ++;
    return num;
  }
  
}
