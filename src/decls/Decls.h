#pragma once

enum BaseClasses
{
    OBJECT,
    INT,
    BOOL,
    STRING,
    IO,
    SELF_TYPE,

    BaseClassesSize
};

extern const char *const BaseClassesNames[BaseClassesSize];

enum ObjectMethods
{
    ABORT,
    TYPE_NAME,
    COPY,

    ObjectMethodsSize
};

extern const char *const ObjectMethodsNames[ObjectMethodsSize];

extern const char *const IntMethodsNames[ObjectMethodsSize];

enum IOMethods
{
    OUT_STRING = COPY + 1,
    OUT_INT,
    IN_STRING,
    IN_INT,

    IOMethodsSize
};

extern const char *const IOMethodsNames[IOMethodsSize];

enum StringMethods
{
    LENGTH = COPY + 1,
    CONCAT,
    SUBSTR,

    StringMethodsSize
};

extern const char *const StringMethodsNames[StringMethodsSize];

extern const char *MainMethodName;

extern const char *MainClassName;

extern const char *SelfObject;