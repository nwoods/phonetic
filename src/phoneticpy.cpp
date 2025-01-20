#include<nanobind/nanobind.h>
#include<nanobind/stl/string.h>
#include<nanobind/stl/vector.h>
#include<nanobind/stl/pair.h>
#include<nanobind/stl/unordered_set.h>

#include "phonetic.hpp"

namespace nb = nanobind;

NB_MODULE(phoneticpy, m)
{
    nb::class_<Phonetic>(m, "Phonetic")
        .def(nb::init<>())
        .def("word_to_phones", &Phonetic::word_to_phones)
        .def("text_to_phones", &Phonetic::text_to_phones)
        .def("phone_to_stress", &Phonetic::phone_to_stress)
        .def("word_to_stresses", &Phonetic::word_to_stresses)
        .def("phone_to_syllable_count", &Phonetic::phone_to_syllable_count)
        .def("word_to_syllable_counts", &Phonetic::word_to_syllable_counts)
        .def("get_rhyming_part", &Phonetic::get_rhyming_part)
        .def("search", &Phonetic::search);
}
