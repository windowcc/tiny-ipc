#ifndef _IPC_CALLBACK_H_
#define _IPC_CALLBACK_H_

#include <memory>
#include <string>

namespace ipc {

class Buffer;

/////////////////////////////////////////////////////////////////////////////

/**
 * @brief Provides a mechanism for tracking the completion of an asynchronous
 * action.
 */
class IPC_EXPORT Callback
{
public:
    /** Smart/shared pointer to an object of this type */
	using ptr_t = std::shared_ptr<Callback>;
	/** Smart/shared pointer to a const object of this type */
	using const_ptr_t = std::shared_ptr<const Callback>;

	virtual ~Callback() {}
public:
    /**
     * @brief Create connection callback function
     * 
     * @param cause 
     */
	virtual void connected(
        const ErrorCode &cause = ErrorCode::IPC_ERR_SUCCESS) {}

    /**
     * @brief Connection loss callback function
     * 
     * @param cause 
     */
	virtual void connection_lost(
        const ErrorCode &cause = ErrorCode::IPC_ERR_SUCCESS) {}

    /**
     * @brief Message arrival callback function
     * 
     */
	virtual void message_arrived(
        const Buffer *buf /*msg*/,
        const ErrorCode &cause = ErrorCode::IPC_ERR_SUCCESS) {}

    /**
     * @brief Message sending callback function
     * 
     * @param cause 
     */
	virtual void delivery_complete(
        const ErrorCode &cause = ErrorCode::IPC_ERR_SUCCESS) {}
};

/** Smart/shared pointer to a callback object */
using CallbackPtr = Callback::ptr_t;

/** Smart/shared pointer to a const callback object */
using ConstCallbackPtr = Callback::const_ptr_t;

} // namespace ipc

#endif // _IPC_CALLBACK_H_