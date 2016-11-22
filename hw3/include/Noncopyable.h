#ifndef __NONCOPYABLE_H
#define __NONCOPYABLE_H
// Classes inheriting Noncopyable will not have default compiler generated copy
// constructor and assignment operator
class Noncopyable {
protected:
  Noncopyable() {}
  ~Noncopyable() {}
private:
  Noncopyable(const Noncopyable&);
  const Noncopyable& operator= (const Noncopyable&);
};
#endif// __NONCOPYABLE_H
