//
// Created on 18.07.2020.
//

#pragma once

namespace hal::info {
    enum class vendors { STM, MICROCHIP };

    template<typename MCU>
    class vendor_information {};

}  // namespace hal::info

template<typename T>
concept input_register_type = requires(T reg) {
    {reg.clear()};
};

template<typename T>
concept output_register_type = requires(T reg) {
    { *reg }
    ->std::integral;
};

template<typename MCU, typename PIN>
concept stm_mcu = requires(MCU::GPIO a) {
    typename MCU::GPIO::template address<PIN>;
    requires input_register_type<decltype(a.moder)>;
    requires input_register_type<decltype(a.otyper)>;
    requires input_register_type<decltype(a.ospeedr)>;
    requires input_register_type<decltype(a.pupdr)>;
    requires output_register_type<decltype(a.idr)>;
    requires input_register_type<decltype(a.bssr_set_io)>;
    requires input_register_type<decltype(a.bssr_clear_io)>;
    requires input_register_type<decltype(a.afr)>;
    requires input_register_type<decltype(a.moder)>;
};

template<typename MCU>
concept mcu_with_vendor_info = requires {
    hal::info::vendor_information<MCU>::vendor;
    std::is_same_v<decltype(hal::info::vendor_information<MCU>::vendor),
                   hal::info::vendors>;  // vendors must be a vendor :P
};

template<typename MCU, typename PIN>
concept specialized_mcu = requires() {
    requires stm_mcu<typename MCU::base_mcu, PIN>;
};

namespace hal::periphery {
    namespace detail {
        template<mcu_with_vendor_info MCU, typename Component>
        requires(hal::info::vendor_information<MCU>::vendor == hal::info::vendors::STM) struct stm_mcu_mapper {};
    }  // namespace detail
}  // namespace hal::periphery
