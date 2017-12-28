#include <type_traits>
#include <utility>
#include <fstream>

namespace impl {
    template <typename Func>
    class function_wrapper {
    public:
        template <typename F>
        function_wrapper(F &&func)
        :m_func{ std::forward<F>(func) }
        {}

        function_wrapper(const function_wrapper&) = delete;
        function_wrapper &operator=(const function_wrapper&) = delete;

        template <typename F>
        inline auto operator()(F &&func) {
            return m_func(std::forward<F>(func));
        }
        
        template <typename F>
        inline auto operator()(F &&func) const {
            return m_func(std::forward<F>(func));
        }

    private:
        Func &&m_func;
    };

    template <typename Func>
    inline function_wrapper<Func&&> wrap_function(Func &&func) {
        return { std::forward<Func>(func) };
    }

    template <typename FF, typename F>
    [[nodiscard]] inline bool operator+(function_wrapper<FF> &&wrapper_func, F &&func) {
        return wrapper_func(std::forward<F>(func));
    }
}

#define SAFE_STREAM(STREAM)             impl::wrap_function([&](auto &&func) { func(); return STREAM.good(); }) + \
                                                  [&]() 
#define SAFE_INLINE_STREAM(STREAM, ...) impl::wrap_function([&](auto &&func) { auto STREAM{ __VA_ARGS__ }; func(STREAM); return STREAM.good(); }) + \
                                                  [&](auto &STREAM)

int main() {
    std::fstream stream;
    // If not captured, warning: ignoring return value 
    bool success_1 = SAFE_STREAM(stream) {
        stream << "abc";
    };

    bool success_2 = SAFE_INLINE_STREAM(strm, std::fstream()) {
        strm << "abc";
    };

    // If not checked, warning: unused variable
    if (!success_1 || !success_2) return 1;
    
    return 0;
}