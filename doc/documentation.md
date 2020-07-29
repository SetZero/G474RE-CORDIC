---
title: 'Entwicklung eines C++ Frameworks zur effizienten Ansteuerung der Periphery eines STM32 Microcontrollers'
date : 27.7.2020
lang: de-De
author:
    - Christian Roth
    - Sebastian Dauenhauer
keywords: [Microcontroller, C++, Type-Safity, Cordic]
link-citations: true
nocite: |
    @*
documentclass: scrartcl
classoption: a4paper, 12pt
fontsize: 12pt
papersize: a4
titlepage: 1
toc: 1
toc-depth: 1
title-own-page: 1
toc-own-page: 1
secnumdepth: 2
abstract: |
    Mikrocontroller erhalten immer wieder neue und stärkere Komponenten, dabei hat sich die Programmierung dieser bisher kaum geändert.
    Durch die neuen Funktionalitäten dieser wird auch die Entwicklung immer komplexer. Mit C++ und seinen Zero Cost Abstractions soll
    deshalb ein Abstraktionslayer entworfen werden, welches die Verwendung der Komponenten einfacher und sicherer machen soll.
    Hierfür wird das Modell G474RE der STM32 Reihe verwendet. Dieser besitzt zusätzlich zu den üblichen Komponenten auch eine Einheit,
    welche trigonometrische Funktionen berechnen kann. Im Zuge dieser Arbeit wird daher auch die Performance dieser Einheit verglichen mit
    den trigonometrischen Funktionen, welche durch Software berechnet werden.
include-before: \newpage
---

# Einleitung

Die Programmierung mit Mikrocontrollern wird immer anspruchsvoller, da diese immer mehr Komponenten und Möglichkeiten erhalten sie zu verwenden.
Gleichzeitig hat sich die in diesem Bereich Softwareentwicklung oftmals kaum weiterentwickelt, sodass Mikrocontroller weiterhin so programmiert werden,
wie es üblich war zu der Zeit, als diese wesentlich weniger komplex waren. C++ mit seinen Zero-Cost Abstractions bietet daher eine sehr gute Möglichkeit,
bestimmte Konzepte zu abstrahieren und somit die Entwicklung für Mikrocontroller nicht nur einfacher, sondern auch sicherer zu gestalten.
Beispielhaft wurde ein Framework für einen STM32 entwickelt. Dabei sollen zwei Komponenten des Mikrocontrollers mit den Mitteln von C++ modelliert werden.
Dafür werden zunächst die zugrunde liegenden Konzepte gezeigt, um dann die Komponente als solche zu modellieren.
Die zweite Komponente, ein sogenannte CORDIC, ist eine spezielle Einheit innerhalb des hier verwendeten Modells des STM32.
Dessen Geschwindigkeit soll mit den bereits eingebauten Trigonometrischen Funktionen verglichen werden.

# Die Ansteuerung der Peripherie

Die Peripherie von Mikrocontrollern und auch bei dem STM32 funktioniert im Allgemeinen über das Setzen von Bits in Registern.
Die Register befinden sich an bestimmten Stellen im Speicher des Mikrocontrollers.
Für das Setzen von Bits würde es genügen einen Pointer mit dem richtigen Typ auf diesen Bereich zeigen zu lassen
und dann die benötigten Bits zu setzen.
Eines der Ziele ist es jedoch neben Typsicherheit eine Resistenz gegen Fehlverwendung herzustellen.
Mit dem Zeiger eines primitiven Datentypes auf des Registers, können unsinnige Werte gesetzt werden, weiterhin gibt es oftmals in
Registern Werte und Teile, die nicht manipuliert werden dürfen.
Um diese beiden Ziele zu erreichen wurde Datentypen implementiert, welche bei der Umsetzung dieser Ziele helfen sollen.

## Beschreibung eines Registers

![Beschreibung des Registers in der Dokumentation des Mikrocontrollers](images/cr1desc.png)

In der vorherigen Abbildung ist die Beschreibung eines Registers des Mikrocontrollers abgebildet, wie sie üblicherweise in einer Dokumentation vorkommt.
Die Bits werden beschrieben, indem diesen eine Funktion zugewiesen wird, ein Bereich und gültige Werte. Weiterhin kann es reservierte Bereiche geben, welche
nicht verändert werden dürfen. Diese Zusammenhänge wurden in einem Datentyp modelliert.

~~~{.cpp}
register_entry_desc<CR::DEDT, uint8_t, bit_range<16u, 20u>, access_mode::read_write>
~~~

Dieser Typ beschreibt die Bitpositionen von 16 - 20 einschließlich, der Bereich kann gelesen und gesetzt werden.
Weiterhin wird die Funktion mit einem enum Eintrag Beschrieben. So muss jedes einzelne Bit beschrieben werden, ein Konzept stellt dies zur Kompilezeit sicher.
Für das UART Register CR ergibt sich damit folgender Typ:

~~~{.cpp}
register_desc<
volatile uint32_t, register_entry_desc<CR::UE, bool, bit_pos<0u>>,
register_entry_desc<CR::UESM, bool, bit_pos<1u>>,
register_entry_desc<CR::RE, bool, bit_pos<2u>>,
register_entry_desc<CR::TE, bool, bit_pos<3u>>,
register_entry_desc<CR::IDLEIE, bool, bit_pos<4u>>,
register_entry_desc<CR::RXNEIE, bool, bit_pos<5u>>,
register_entry_desc<CR::TCIE, bool, bit_pos<6u>>,
register_entry_desc<CR::TXEIE, bool, bit_pos<7u>>,
register_entry_desc<CR::PEIE, bool, bit_pos<8u>>,
register_entry_desc<CR::PS, parity, bit_pos<9u>>,
register_entry_desc<CR::PCE, bool, bit_pos<10u>>,
register_entry_desc<CR::WAKE, bool, bit_pos<11u>>,
register_entry_desc<CR::M0, bool, bit_pos<12u>>,
register_entry_desc<CR::MME, bool, bit_pos<13u>>,
register_entry_desc<CR::CMIE, bool, bit_pos<14u>>,
register_entry_desc<CR::OVER8, bool, bit_pos<15u>>,
register_entry_desc<CR::DEDT, uint8_t, bit_range<16u, 20u>>,
register_entry_desc<CR::DEAT, uint8_t, bit_range<21u, 25u>>,
register_entry_desc<CR::RTOIE, bool, bit_pos<26u>>,
register_entry_desc<CR::EOBIE, bool, bit_pos<27u>>,
register_entry_desc<CR::M1, bool, bit_pos<28u>>,
register_entry_desc<CR::FIFOEN, bool, bit_pos<29u>>,
register_entry_desc<CR::RESERVED, reserved_type, bit_range<30u, 31u>>>
~~~

Auf diese Weise können auch Register modelliert werden, welche Werte akzeptieren, also keine Steuerungsregister sind.
Hier muss ebenfalls jedes einzelne Bit beschrieben werden.

~~~{.cpp}
register_desc<volatile uint32_t, register_entry_desc<0, uint8_t, bit_range<0u, 7u>>,
                          register_entry_desc<1, uint8_t, bit_range<8u, 31u>, access_mode::no_access>>
~~~

Bei diesem Register sind nur die ersten 8 Bits beschreibbar.

## Platzierung eines Registers

Wie eingangs erwähnt, sind Register bestimmte Stellen im Speicher. Damit muss die vorherige gezeigte Beschreibung eines Register korrekt platziert werden.
Oftmals gibt es auch mehrere Register der selben Art, welche nur an anderen Positionen stehen. Daher braucht man eine Lösung die Register an die richtigen Stellen
im Code zu platzieren.

~~~{.cpp}
template<typename Component, auto N>
[[nodiscard]] constexpr inline auto address() {
    return reinterpret_cast<Component*>(Component::template address<N>::value);
}

template<typename Component, typename N>
[[nodiscard]] constexpr inline auto address() {
    return reinterpret_cast<Component*>(Component::template address<N>::value);
}
~~~

Die beiden Methoden werden dann parametrisiert mit der gewünschten Peripherie oder hier Component genannt, mit dem entsprechenden Index.
Die Funktion greift dafür auf einen zuvor festgelegten Bereich zu und wandelt, diesen in den gewünschten Datentyp um.
Die Beschreibung dieser Zusammenhänge werden in structs gespeichert, welche den speziellen Mikrocontroller beschreiben, diese wird nachfolgend erläutert

## Beschreibung des Mikrocontroller Aufbaus

Die Beschreibung eines Mikrocontrollers ist die Anzahl und die Art seiner Komponenten.
Dies kann man mit einem Konzept implementieren, was die Fähigkeiten eines Mikrocontrollers modellieren kann.

~~~{.cpp }
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
~~~

Obiges Beispiel modelliert einen Mikrocontroller mit gpios, damit kann sichergestellt werden, dass eine konkrete Implementierung für GPIOs vorliegt.
Diese Concepts werden im Zusammenhang mit dem HAL (Hardware-abstraction-layer) verwendet, dieses verallgemeinert eine Implementierung für mehrere Mikrocontroller.

## Hardware Abstraction Layer

### UART

### GPIO

### CORDIC

# CORDIC

In folgendem Abschnitt wird zunächst die konkrete Implementierung vorgestellt, welche die CORDIC Einheit ansteuert.
Danach wird die Performance dieser Einheit mit den in gcc eingebauten trigonometrischen Funktionen verglichen.

## Konkrete Implementierung der Cordicverwendung und zugehörige Datentypen

Wie eingangs erwähnt liegt die Stärke von C++ vor allem in den Zero-Cost-Abstractions, welche eine Typsicherheit herstellen.
Für die CORDIC Komponente wurden deswegen eigene Datentypen entworfen die, dies bieten können.
Die CORDIC Komponente wird benutzt, indem man die gewünschte Einstellungen im Kontrollregister vornimmt,
dann je nach Anzahl von Argumenten diese nacheinander in die Argumentregister schreibt.
Nachdem der Algorithmus durchlaufen ist, werden die Ergebnisse je nach Anzahl wieder ausgelesen.
Die Werte, welche in das Argumentregister geschrieben werden und die Ergebnisse, welche man aus dem Ergebnisregister erhält sind keine Werte, welche man direkt verwenden kann.
Sie sind in einem Festkomma Format, dabei gibt es zwei Varianten *q1_31* und *q1_15*.
Die Zahlen bewegen sich somit in einem Interval zwischen -1.0 und 1.0.  Größere Werte müssen durch die Software skaliert werden.
Auch haben einige Funktionen unterschiedliche Definitionsbereiche, so müssen diese ebenfall in den Datentyp miteinbezogen werden.

## Zeitmessung und Vergleich mit eingebauten Trigonometrischen Funktionen

# Fazit

Es konnte gezeigt werden, dass C++ gut verwendet werden kann, um typsichere und effiziente Abstraktionen zu erstellen, die die Komplexität bei der Verwendung von Mikrocontrollern
mindern kann. Dabei ist nicht nur hilfreich bei der Verwendung der Abstraktionslayer, sondern unterstützt ebenfalls die Erstellung weiterer Abstraktionen.

## Ausblick

Durch den weiteren Ausbau des Frameworks können die anderen Peripherien des Mikrocontrollers typsicher und einfach verwendet werden.
Zudem kann diese Bibliothek auf andere Mikrocontroller portiert werden, sodass man eine gemeinsame Schnittstelle verwenden kann.
Es können weitere Operationen für die q_number Klasse hinzugefügt werden, sodass die etwas teure Umwandlung von FließKommazahlen in Fixed-Kommazahlen
erspart bleibt und man stattdessen mit den diesem Typen weiter rechnen kann.
Weiterhin könnte die CORDIC-Einheit im Pipeline Modus gut mit dem ranges feature von C++20 verbunden werden.
Dadurch kann eine gewohnte API, auch für die CORDIC Einheit verwendet werden kann.
Abschließend kann gesagt werden, dass die Verwendung von C++ auf Mikrocontrollern sinnvoll ist, da die Entwicklung vereinfacht wird und häufige Fehler durch
C++ features bereits zur Kompilezeit aufgedeckt werden können.
Somit können viele Tests bereits zur Kompilezeit stattfinden, die sonst umständlich auf dem Mikrocontroller durchgeführt werden müssten.
