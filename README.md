//Если что - Zavar30, Zavar, Abuzov Yaroslav - это все я (нужно будет обновить имя в ide)
Реализация модуля для работы с файлом, который хранит данные, образующие дерево. Сборка будет осуществляться с помощью cmake.

Что уже сделано:
1) Архитектура системы (см. idea.pdf)
2) Продумана реализация некоторых функций и элементов модуля.
3) Реализованы структуры для документа, элемента документа, индексов, схемы, заголовка файла и пр.
4) Реализована структура данных для хранения и выдачи пустых индексов (см. архитектуру, aka свободное место в файле)
5) Реализованы функции для загрузки и создания файла, а также для изменения его заголовка.
6) Создание/удаление индексы в файле
7) Работа со схемой
8) Работа с индексами через mmap
9) Создание "пустого" файла 
10) Сохранение и восстановление списка пустых индексов в/из файла
11) Исправлена проблема с разным размером структур на разных платформах (теперь файл будет выглядеть одинаково везде)
12) Переезд на новую архитектуру v2 (см. idea.pdf)
13) В файле теперь храниться его размер
14) Переезд на fseeko и на off_t для работы с большими файлами
15) Обход дерева (~~пока рекурсивно~~ итеративно)
16) Предикат для поиска
17) Список результатов
18) Создание документа (и привязка его к родителю)
19) Реализована десериалиазация свойств
20) Реализована сериализация свойств
21) Буферное чтение элементов документа во время поиска
22) Удаление документа
23) Обновление элементов документов
24) Итератор для чтения элементов документа
25) Оптимизация строк: toast таблицы, строки хранятся в чанках (концепт)
26) Перенос подсчёт размера элементов документа в схему (теперь нужен один проход во время создания)
27) При обновлении есть проверка на типы (если тип не соотв. - будет возвращена ошибка)
28) Итератор для документов
29) Работа со строчками через toast секции и чанки (реализация)
30) Join (возвращает всех детей документа)
31) Исправлен баг с удалением из корня (переписано удаление)
32) Index expand
33) Куча мелких вещей

В ближайших планах:
1) Создать структуры для запросов (подготовиться к XPath, вставить проверку элементов в findIf и в forEach)
2) По мелочи - убрать лишний вывод и т.д.

