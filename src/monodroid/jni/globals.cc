#include "globals.h"

using namespace xamarin::android;
using namespace xamarin::android::internal;

DylibMono monoFunctions;
Util utils;
AndroidSystem androidSystem;
OSBridge osBridge;
EmbeddedAssemblies embeddedAssemblies;
InMemoryAssemblies inMemoryAssemblies;

#ifdef DEBUG
Debug debug;
#endif
