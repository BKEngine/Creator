#include <QtGlobal>

#if defined(Q_OS_LINUX)
#include "src/worker/linux/cookie_jar.cpp"
#include "src/worker/linux/linux_worker_platform.cpp"
#include "src/worker/linux/pipe.cpp"
#include "src/worker/linux/side_effect.cpp"
#include "src/worker/linux/watch_registry.cpp"
#include "src/worker/linux/watched_directory.cpp"
#include "src/helper/common_posix.cpp"
#elif defined(Q_OS_MAC)
#include "src/worker/macos/batch_handler.cpp"
#include "src/worker/macos/macos_worker_platform.cpp"
#include "src/worker/macos/rename_buffer.cpp"
#include "src/worker/macos/subscription.cpp"
#include "src/helper/macos/helper.cpp"
#include "src/helper/common_posix.cpp"
#elif defined(Q_OS_WIN)
#include "src/helper/common_win.cpp"
#include "src/helper/windows/helper.cpp"
#include "src/worker/windows/subscription.cpp"
#include "src/worker/windows/windows_worker_platform.cpp"
#endif
