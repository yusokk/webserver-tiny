C 언어를 이용한 Tiny Lab (소형 웹 서버)

- 참여자 : 김용욱, 김유석, 최성현

- 프로젝트 기간 : 2021.01.22 ~ 2021.01.28

- Tiny 웹 서버 : 

  * 프로세스 제어, Unix I/O, 소켓 인터페이스, HTTP 등의 개념을 결합한 소형 웹 서버

  * 실제 웹 브라우저에 정적 및 동적 컨텐츠 제공

    

- 네트워크 요약

- 모든 네트워크 애플리케이션은 클라이언트 - 서버 모델에 기초

  - 애플리케이션은 한 개의 서버와 한 개 이상의 클라이언트로 구성
  - 서버는 자원을 관리, 자원을 조작해서 클라이언트에게 서비스 제공
  - 클라이언트 - 서버 모델의 기본 연산 : 클라이언트 서버 트랜잭션, 클라이언트의 요청과 이에 대한 서버의 응답으로 이루어짐

- 클라이언트와 서버는 인터넷이라는 글로벌 네트워크를 통해서 통신

  - 각 인터넷 호스트는 IP 주소라는 고유한 32비트 이름을 가짐
  - IP 주소 집합은 인터넷 도메인이름의 집합으로 대응
  - 서로 다른 인터넷 호스트에서의 프로세스들은 연결을 통해 서로 통신

- 클아이언트와 서버는 소켓 인터페이스를 사용해서 연결 수립

  - 소켓은 연결의 끝점이며 애플리케이션에게는 파일 식별자의 형태로 제공
  - 소켓 인터페이스는 소켓 식별자를 열고 닫기 위한 함수들을 제공
  - 클라이언트와 서버는 이 식별자들을 서로 읽고 쓰는 방식으로 통신

- 웹 서버와 이들의 클라이언트들은 HTTP 프로토콜을 사용해서 서로 통신

  - 브라우저는 서버로부터 정적 또는 동적 컨텐츠를 요청
  - 정적 컨텐츠를 위한 요청은 서버의 디스크에서 파일을 가져와 이것을 클라이언트에 돌려주는 방식으로 처리
  - 동적 컨텐츠에 대한 요청은 서버에서 자식 프로세스의 컨텍스트에서 프로그램을 돌리고, 그 출력을 클라이언트로 리턴해서 처리