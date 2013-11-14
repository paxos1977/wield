#pragma once

#ifdef _WIN32

    // Use Microsoft's concurrent_queue implementation
    // Disable some warnings
    #pragma warning(push)
    #pragma warning(disable : 4127 4625 4626)
    #include <concurrent_queue.h>
    #pragma warning(pop)

#else
    // Use Intel Thread Build Blocks concurrent_queue
    #include <tbb/concurrent_queue.h>
#endif
