#pragma once

#include "ObjectLayout.hpp"

extern "C"
{
    /**
     * @brief Initialize language runtime
     *
     * @param argc Number of args in argv
     * @param argv Argument strings
     */
    void _init_runtime(int argc, char **argv); // NOLINT

    /**
     * @brief Finalize runtime
     *
     */
    void _finish_runtime(); // NOLINT

    /**
     * @brief Check if two objects are equal
     *
     * @param lo left operand
     * @param ro right operand
     * @return TrueVal for true, FalseVal for false
     */
    int _equals(ObjectLayout *lo, ObjectLayout *ro); // NOLINT

    /**
     * @brief Abort on case mismatch
     *
     * @param tag Class tag
     */
    void _case_abort(int tag); // NOLINT

    /**
     * @brief Abort on dispatch to void
     *
     * @param filename File name of the method
     * @param linenumber Line number of the method
     */
    void _dispatch_abort(StringLayout *filename, int linenumber); // NOLINT

    /**
     * @brief Abort on match on void
     *
     * @param filename File name of the method
     * @param linenumber Line number of the method
     */
    void _case_abort_2(StringLayout *filename, int linenumber); // NOLINT

    // Basic classes methods
    // ------------------------------ Object ------------------------------
    /**
     * @brief Abort program with debuf info
     *
     * @param receiver Receiver
     * @return NULL
     */
    ObjectLayout *Object_abort(ObjectLayout *receiver); // NOLINT

    /**
     * @brief Get type of the Object
     *
     * @param receiver Receiver
     * @return Cool String
     */
    StringLayout *Object_type_name(ObjectLayout *receiver); // NOLINT

    /**
     * @brief Get copy of the Object
     *
     * @param receiver Receiver
     * @return Copy of the Object
     */
    ObjectLayout *Object_copy(ObjectLayout *receiver); // NOLINT

    // ------------------------------ String ------------------------------

    /**
     * @brief Cool String length
     *
     * @param receiver Receiver
     * @return Cool Int for length
     */
    IntLayout *String_length(StringLayout *receiver); // NOLINT

    /**
     * @brief Cool String concat
     *
     * @param receiver Receiver
     * @param str Other Cool String
     * @return New Cool String
     */
    StringLayout *String_concat(StringLayout *receiver, StringLayout *str); // NOLINT

    /**
     * @brief Cool String substring
     *
     * @param receiver Receiver
     * @param index Starting index
     * @param len Substring length
     * @return New Cool String
     */
    StringLayout *String_substr(StringLayout *receiver, IntLayout *index, IntLayout *len); // NOLINT
    // ------------------------------ IO ------------------------------
    /**
     * @brief Print Cool String
     *
     * @param receiver Receiver
     * @param str Cool string for printing
     * @return Receiver
     */
    ObjectLayout *IO_out_string(ObjectLayout *receiver, StringLayout *str); // NOLINT

    /**
     * @brief Print Cool Int
     *
     * @param receiver Receiver
     * @param integer Cool Int for printing
     * @return Receiver
     */
    ObjectLayout *IO_out_int(ObjectLayout *receiver, IntLayout *integer); // NOLINT

    /**
     * @brief Read line to Cool String
     *
     * @param receiver Receiver
     * @return New Cool String
     */
    StringLayout *IO_in_string(ObjectLayout *receiver); // NOLINT

    /**
     * @brief Read line to Cool Int
     *
     * @param receiver Receiver
     * @return New Cool Int
     */
    IntLayout *IO_in_int(ObjectLayout *receiver); // NOLINT

    // ------------------------------ GC ------------------------------
    /**
     * @brief Allocate object with known size
     *
     * @param tag Object tag
     * @param size Object size
     * @param disp_tab Dispatch table
     * @return Pointer to the newly allocated object
     */
    ObjectLayout *_gc_alloc(int tag, size_t size, void *disp_tab); // NOLINT

#ifdef DEBUG
    /**
     * @brief Check if the obj is a heap object
     *
     * @param obj Object pointer
     */
    void _verify_oop(ObjectLayout *obj); // NOLINT

#endif // DEBUG
}
