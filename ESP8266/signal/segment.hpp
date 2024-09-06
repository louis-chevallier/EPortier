
struct Segment {
  virtual float duration_ms() const { return 0; }
  virtual float data(float t_ms) const { return 0; }
};

struct Ramp : Segment {
  const float duration, begin, end;
 Ramp(float _duration, float _begin, float _end) :
  begin(_begin),
    end(_end),
    duration(_duration)
      {}
  float duration_ms() const { return duration; }
  float data(float t_ms) const {
    auto
      slope = (end - begin)/duration_ms(),
      d = t_ms - begin;
    return d*slope;
  }
};

template<class T> struct Rev {
  const T r;
  Rev(const T &_r) : r(_r) {
  }
  float duration_ms() const { return r.duration_ms(); }
  float data(float t_ms) const { return r.data(duration_ms() - t_ms); }
};

template<class T1, class T2> struct Cat {
  const T1 r1;
  const T2 r2;
  Cat(const T1 &_r1, const T2 &_r2) : r1(_r1), r2(_r2) {
  }
  float duration_ms() const { return r1.duration_ms() + r2.duration_ms(); }
  float data(float t_ms)  const {
    return t_ms > r1.duration_ms() ? r2.data(t_ms - r1.duration_ms()) : r1.data(t_ms);
  }
};

template<class T> struct Repeat {
  const T r;
  const int n;
  Repeat(const T &_r, int _n) : r(_r), n(_n) {}
  float duration_ms()  const { return r.duration_ms() * n; }
  float data(float t_ms)  const {
    auto k = t_ms / r.duration_ms();
    int ki = int(k);
    auto s = t_ms - ki*r.duration_ms();
    //printf("r dur %f t %f k %f ki %d s %f\n", r.duration_ms(), t_ms, k, ki, s);
    return r.data(s);
  }
};

template<class T> const Repeat<T> repeat(const T &r, int n) { return Repeat<T>(r, n);}
template<class T1, class T2> const Cat<T1, T2> cat(const T1 &r1, const T2 &r2) { return Cat<T1, T2>(r1, r2); }
template<class T> const Rev<T> rev(const T &r) { return Rev<T>(r); }
