#ifdef NANOGUI_PYTHON

#include "python.h"

typedef IntBox<int64_t> Int64Box;

void register_formhelper(nb::module_ &m) {
    enum DummyEnum { };

    nb::class_<FormHelper>(m, "FormHelper", D(FormHelper))
        .def(nb::init<Screen *>(), D(FormHelper, FormHelper))
        .def(nb_method(FormHelper, add_window), "pos"_a,
             "title"_a = std::string("Untitled"),
             D(FormHelper, add_window))
        .def(nb_method(FormHelper, add_group), D(FormHelper, add_group))
        .def(nb_method(FormHelper, add_button), "label"_a,
             "cb"_a, D(FormHelper, add_group))
        .def("add_bool_variable",
             [](FormHelper &h, const std::string &label,
                const std::function<void(const bool &) > &setter,
                const std::function<bool(void) > &getter, bool editable) -> CheckBox* {
                return h.add_variable(label, setter, getter, editable);
             },
             "label"_a, "setter"_a, "getter"_a,
             "editable"_a = true)
        .def("add_int_variable",
             [](FormHelper &h, const std::string &label,
                const std::function<void(const int64_t &) > &setter,
                const std::function<int64_t(void) > &getter, bool editable) -> Int64Box* {
                return h.add_variable(label, setter, getter, editable);
             },
             "label"_a, "setter"_a, "getter"_a,
             "editable"_a = true)
        .def("add_double_variable",
             [](FormHelper &h, const std::string &label,
                const std::function<void(const double &) > &setter,
                const std::function<double(void) > &getter, bool editable) -> FloatBox<double>* {
                return h.add_variable(label, setter, getter, editable);
             },
             "label"_a, "setter"_a, "getter"_a,
             "editable"_a = true)
        .def("add_string_variable",
             [](FormHelper &h, const std::string &label,
                const std::function<void(const std::string &) > &setter,
                const std::function<std::string(void) > &getter, bool editable) -> TextBox* {
                return h.add_variable(label, setter, getter, editable);
             },
             "label"_a, "setter"_a, "getter"_a,
             "editable"_a = true)
        .def("add_color_variable",
             [](FormHelper &h, const std::string &label,
                const std::function<void(const Color &) > &setter,
                const std::function<Color(void) > &getter, bool editable) -> ColorPicker* {
                return h.add_variable(label, setter, getter, editable);
             },
             "label"_a, "setter"_a, "getter"_a,
             "editable"_a = true)
        .def("add_enum_variable",
             [](FormHelper &h, const std::string &label,
                const std::function<void(const int &) > &setter,
                const std::function<int(void) > &getter, bool editable) -> ComboBox* {
                return h.add_variable(label,
                        reinterpret_cast<const std::function<void(const DummyEnum &)>&>(setter),
                        reinterpret_cast<const std::function<DummyEnum(void)>&>(getter),
                        editable);
             },
             "label"_a, "setter"_a, "getter"_a,
             "editable"_a = true)
        .def(nb_method(FormHelper, add_widget), D(FormHelper, add_widget))
        .def(nb_method(FormHelper, refresh), D(FormHelper, refresh))
        .def(nb_method(FormHelper, window), D(FormHelper, window))
        .def(nb_method(FormHelper, set_window), D(FormHelper, set_window))
        .def(nb_method(FormHelper, fixed_size), D(FormHelper, fixed_size))
        .def(nb_method(FormHelper, set_fixed_size), D(FormHelper, set_fixed_size))
        .def(nb_method(FormHelper, group_font_name), D(FormHelper, group_font_name))
        .def(nb_method(FormHelper, set_group_font_name), D(FormHelper, set_group_font_name))
        .def(nb_method(FormHelper, label_font_name), D(FormHelper, label_font_name))
        .def(nb_method(FormHelper, set_label_font_name), D(FormHelper, set_label_font_name))
        .def(nb_method(FormHelper, group_font_size), D(FormHelper, group_font_size))
        .def(nb_method(FormHelper, set_group_font_size), D(FormHelper, set_group_font_size))
        .def(nb_method(FormHelper, label_font_size), D(FormHelper, label_font_size))
        .def(nb_method(FormHelper, set_label_font_size), D(FormHelper, set_label_font_size))
        .def(nb_method(FormHelper, widget_font_size), D(FormHelper, widget_font_size))
        .def(nb_method(FormHelper, set_widget_font_size), D(FormHelper, set_widget_font_size));
}
#endif
