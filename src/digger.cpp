
#include "digger.h"
#include "message.h"
#include "question.h"

#include <future>
#include <iostream>

using namespace dns;

std::future<Message> Digger::dig(Question question) const
{
    (void)question;
    std::cout << "TODO!" << std::endl;
    std::promise<Message> p;
    p.set_value(Message{});
    return p.get_future();
}
