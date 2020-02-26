#include <iostream>
#include <string>
#include <thread>
#include <future>

#include "flib/WaitQueue.hpp"

int main()
{
  flib::WaitQueue<std::string> queue;
  queue.Push("from thread1");
  queue.Push("Hello", 1);
  queue.Push("World", 1);
  std::async(std::launch::async, [&queue]()
    {
      try
      {
        while (true)
        {
          std::cout << queue.WaitedPop(std::chrono::milliseconds(50)) << '\n';
        }
      }
      catch (const std::exception & e)
      {
        std::cout << e.what() << '\n';
      }
      std::cout << "thread1 has ended\n";
    }
  ).get();
  queue.Push("from thread2");
  queue.Push("World", 1);
  queue.Push("Hello", 2);
  std::async(std::launch::async, [&queue]()
    {
      try
      {
        while (true)
        {
          std::cout << queue.Pop() << '\n';
        }
      }
      catch (const std::exception & e)
      {
        std::cout << e.what() << '\n';
      }
      std::cout << "thread2 has ended\n";
    }
  ).get();
  return 0;
}