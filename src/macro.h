#ifndef MACRO_H
#define MACRO_H

#ifdef SIMULATOR_HOST
#define FORMAT_SIZE(buf, size) snprintf(buf, sizeof(buf), "%llu", (uint64_t)size)
#else
#define FORMAT_SIZE(buf, size) snprintf(buf, sizeof(buf), "%u", (unsigned int)size)
#endif

#endif