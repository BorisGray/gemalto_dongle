#include <iostream>
//#include <jni.h>
using namespace std;

extern void getDongleInfo();

int main(int argc, char *argv[])
{
  cout << "Hello World!" << endl;

  getDongleInfo();

  return 0;
}
