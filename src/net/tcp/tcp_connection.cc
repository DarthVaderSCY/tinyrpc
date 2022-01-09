#include <unistd.h>
#include <string.h>
#include "tcp_connection.h"

namespace tinyrpc {

TcpConection::TcpConection(tinyrpc::Reactor* reactor) : FdEvent(reactor), m_state(Connected) {	
  
}

TcpConection::~TcpConection() {

}

void TcpConection::init(int fd, int size) {

  // 初始化缓冲区大小
  m_write_buffer = std::make_shared<TcpBuffer>(size);
  m_read_buffer = std::make_shared<TcpBuffer>(size);

  setFd(fd);

	addListenEvents(IOEvent::READ);
	// addListenEvents(IOEvent::WRITE);
  
  setCallBack(IOEvent::READ, std::bind(&TcpConection::onReadEvent, this));

  setCallBack(IOEvent::WRITE, std::bind(&TcpConection::onWriteEvent, this));

  // 注册读写监听到reactor
  updateToReactor();

}

void TcpConection::onReadEvent() {
	// copy data from socket to app buffer
	
	if (m_read_buffer->writeAble() == 0) {
		m_read_buffer->resize(2 * m_read_buffer->getSize());	
	}
	int read_count = m_read_buffer->writeAble();
	char buf[read_count];

	int rt = read(m_fd, &buf, read_count);		
	if (rt == -1) {
		ErrorLog << "read empty, error=" << strerror(errno);
	} else {
		DebugLog << "succ read " << rt << " bytes: " << buf << ", count=" << rt;
	}
	m_read_buffer->writeToBuffer(buf, rt);

}

void TcpConection::onWriteEvent() {
	std::vector<char> re;
	m_write_buffer->readFromBuffer(re, m_write_buffer->readAble());
	char* buf = &re[0];	

	int rt = write(m_fd, buf, re.size());
	if (rt == -1) {
		ErrorLog << "write empty, error=" << strerror(errno);
	} else {
		DebugLog << "succ write " << rt << " bytes: " << buf;
	}
}

void TcpConection::asyncRead(std::vector<char>& re, int size) {
	m_read_buffer->readFromBuffer(re, size);	
}

void TcpConection::asyncWrite(const std::vector<char>& buf) {
	const char* tmp = &buf[0];
	int size = buf.size();
	m_write_buffer->writeToBuffer(tmp, size);
}

}
