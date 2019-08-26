#include <exception>
#include <string>

class WNException : public std::exception
{
	public:
		WNException(const std::string& message) :
			m_message(message)
		{

		}

		const char* what() const throw ()
		{
			return m_message.data();
		}
	private:
		std::string m_message;
};

class WNDisconnectException : public WNException
{
	public:
		WNDisconnectException(const std::string& message) :
			WNException(message)
		{

		}
};

class WNInvalidOperationException: public WNException
{
	public:
		WNInvalidOperationException(const std::string& message) :
			WNException(message)
		{

		}
};

class WNManagerException : public WNException
{
	public:
	WNManagerException(const std::string& message) :
		WNException(message)
	{

	}
};