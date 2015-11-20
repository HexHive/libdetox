#include <iostream>
#include <exception>
using namespace std;

class A {
public:
  A() {}
  ~A() {}
};

class myexception: public exception
{
  virtual const char* what() const throw() /* means no exception will be thrown by this method, required as we override what() as defined by class exception */
  {
    return "My exception happened";
  }
} myex;

int method1 () {
 int i;
 A a;
 i = 0;
 throw myex;
 i++;
 return 0;
}

int method2 () {
 int j;
 j = 0;
 method1 ();
 j++;
 return 0;
}

int main () {
  int k;
  try
  {
    k = 0;
    method2();
    k++; 
  }
  catch (exception& e)
  {
    cout << e.what() << '\n';
  }
  return 0;
}

