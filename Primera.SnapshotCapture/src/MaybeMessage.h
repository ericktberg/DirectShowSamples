#pragma once
#include <string>

/// <summary>
/// I know that it's a convention in C and C++, but I've never really been comfortable conditionally assigning to passed pointers.
/// If I am getting an object and it could potentially be uninitiated afterwards, I'd rather have a concrete type encoding that.
/// <para/>
/// Enter MaybeMessage which can encode both type information and the error message in a binary fashion
/// </summary>
/// <typeparam name="T"></typeparam>
template <typename T>
class MaybeMessage
{
public:
    MaybeMessage(T* result, std::string errorMessage);

    T* getResult();
    bool isError();
    std::string getErrorMessage();

private:
    T* _result;
    std::string _errorMessage;
};

/*
* IMPLEMENTATION
*
* Something about how the linker works, I have to include the entire template type definition in one file (?)
* https://stackoverflow.com/questions/12848876/unresolved-external-symbol-template-class
*
*/

template<typename T>
MaybeMessage<T>::MaybeMessage(T* result, std::string errorMessage)
{
    _result = result;
    _errorMessage = errorMessage;
}

template<typename T>
T* MaybeMessage<T>::getResult()
{
    return isError() ? nullptr : _result;
}

template<typename T>
bool MaybeMessage<T>::isError()
{
    return !_errorMessage.empty();
}

template<typename T>
inline std::string MaybeMessage<T>::getErrorMessage()
{
    return _errorMessage;
}

/*
*  FACTORY METHODS
*
*  We need to include 2 parameters into the constructor of any maybe type.
*  Let's simplify calling code by making all permutations of our type into simpler methods.
*/

template<typename T>
MaybeMessage<T> maybeFail(std::string errorMessage) {
    return MaybeMessage<T>(nullptr, errorMessage);
}

template<typename T>
MaybeMessage<T> maybeSuccess(T* result) {
    return MaybeMessage<T>(result, "");
}
