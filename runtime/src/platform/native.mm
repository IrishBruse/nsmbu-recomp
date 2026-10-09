
#import <Foundation/Foundation.h>
#include "host.h"
#include <exception>

namespace host {
void with_autorelease_pool(void (*fn)(void*), void* context) {

  std::exception_ptr error;
  @autoreleasepool {
    try { fn(context); }
    catch (...) { error = std::current_exception(); }
  }
  if (error) std::rethrow_exception(error);
}
void with_autorelease_pool(void (*fn)()) {
  with_autorelease_pool([](void* context) {
    (*static_cast<void (**)()>(context))();
  }, &fn);
}
}
