#ifndef SYS_ARCH_H
#define SYS_ARCH_H

namespace inet {

namespace tcp {

typedef void* sys_sem_t;
typedef void* sys_mbox_t;
typedef void* sys_thread_t;

#define SYS_SEM_NULL nullptr
#define SYS_MBOX_NULL nullptr

} // namespace tcp

} // namespace inet

#endif //SYS_ARCH_H

