#include "coordinator.h"

#include "config.h"
#include "json.hpp"
#include "message.h"
#include "session.h"

namespace nvds {

using nlohmann::json;

Coordinator::Coordinator()
    : BasicServer(kCoordPort) {
}

void Coordinator::Run() {
  // TODO(wgtdkp): initializations

  Accept(std::bind(&Coordinator::HandleRecvMessage, this,
                   std::placeholders::_1, std::placeholders::_2),
         std::bind(&Coordinator::HandleSendMessage, this,
                   std::placeholders::_1, std::placeholders::_2));
  RunService();
}

void Coordinator::HandleRecvMessage(std::shared_ptr<Session> session,
                                    std::shared_ptr<Message> msg) {
  NVDS_LOG("received a message");
  switch (msg->sender_type()) {
  case Message::SenderType::SERVER:
    HandleMessageFromServer(session, msg);
    break;
  case Message::SenderType::CLIENT:
    HandleMessageFromClient(session, msg);
    break;
  case Message::SenderType::COORDINATOR:
    assert(false);
  }
}

void Coordinator::HandleSendMessage(std::shared_ptr<Session> session,
                                    std::shared_ptr<Message> msg) {
  // Start again
  session->Start();
}

void Coordinator::HandleMessageFromServer(std::shared_ptr<Session> session,
                                          std::shared_ptr<Message> msg) {
  NVDS_LOG("received a message from server");
  switch (msg->type()) {
  case Message::Type::REQ_JOIN:
    NVDS_LOG("join message: %s", msg->body().c_str());
    HandleServerRequestJoin(session, msg);
    break;
  case Message::Type::REQ_LEAVE:
    break;
  case Message::Type::ACK_REJECT:
    break;
  case Message::Type::ACK_ERROR:
    break;
  case Message::Type::ACK_OK:
    break;
  default:
    assert(false);
  }
}

void Coordinator::HandleServerRequestJoin(std::shared_ptr<Session> session, 
                                          std::shared_ptr<Message> req) {
  auto body = json::parse(req->body());
  uint64_t nvm_size = body["size"];

  NVDS_LOG("join request from server: [%s]", session->GetPeerAddr().c_str());

  index_manager_.AddServer(session->GetPeerAddr(), body["ib_addr"]);
  sessions_.push_back(session);
  ++num_servers_;
  total_storage_ += nvm_size;

  assert(num_servers_ <= kNumServers);
  if (num_servers_ == kNumServers) {
    NVDS_LOG("all servers' join request received. [total servers = %d]",
             kNumServers);
    index_manager_.AssignTablets();
    ResponseAllJoins();
  }
}

void Coordinator::ResponseAllJoins() {
  assert(sessions_.size() == kNumServers);

  for (size_t i = 0; i < sessions_.size(); ++i) {
    json j_body {
      {"id", i},
      {"index_manager", index_manager_}
    };
    sessions_[i]->AsyncSendMessage(std::make_shared<Message>(
        Message::Header {Message::SenderType::COORDINATOR,
                         Message::Type::RES_JOIN},
        j_body.dump()));
  }
  sessions_.clear();
}

void Coordinator::HandleMessageFromClient(std::shared_ptr<Session> session,
                                          std::shared_ptr<Message> msg) {
  switch (msg->type()) {
  case Message::Type::REQ_JOIN:
    break;
  case Message::Type::REQ_LEAVE:
    break;
  case Message::Type::ACK_REJECT:
    break;
  case Message::Type::ACK_ERROR:
    break;
  case Message::Type::ACK_OK:
    break;
  default:
    assert(false);
  }
}

void Coordinator::HandleClientRequestJoin(std::shared_ptr<Session> session,
                                          std::shared_ptr<Message> msg) {
  assert(msg->sender_type() == Message::SenderType::CLIENT);
  assert(msg->type() == Message::Type::REQ_JOIN);
  NVDS_LOG("join request from client: [%s]", session->GetPeerAddr().c_str());
  assert(false);
}

} // namespace nvds
