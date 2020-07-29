#pragma once

template<const char BenchmarkName[], typename ResetCounterFunc, typename GetCounterFunc, typename CounterType>
class benchmark_probe {
   public:
    benchmark_probe(ResetCounterFunc reset_func, GetCounterFunc counter_func, CounterType *count_store)
        : m_counter(count_store), m_counter_func(counter_func) {
        reset_func();
    }
    ~benchmark_probe() { *m_counter = static_cast<CounterType>(m_counter_func()); }

   private:
    CounterType *m_counter;
    GetCounterFunc m_counter_func;
};

template<typename ResetCounterFunc, typename GetCounterFunc>
class benchmark {
   public:
    benchmark(ResetCounterFunc reset_func, GetCounterFunc counter_func)
        : m_reset_func(reset_func), m_counter_func(counter_func) {}

    template<const char BenchmarkName[], typename CounterType>
    auto create_probe(CounterType *count_store) {
        return benchmark_probe<BenchmarkName, ResetCounterFunc, GetCounterFunc, CounterType>{
            m_reset_func, m_counter_func, count_store};
    }

   private:
    ResetCounterFunc m_reset_func;
    GetCounterFunc m_counter_func;
};
