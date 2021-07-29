# Flash Based SSD



[TOC]

---



### 1. Flash



본 글에서는 FTL 을 구현하기 앞서 Flash 및 SSD에대한 전반적인 개념정리를 다룬다.



반도체공학에서 **FLASH** 는 `EEPROM(Electrically Erasable/Programmable Read Only Memory)`의 일종으로 Byte 단위의 지우기작업을 하는 기존의 EEPROM과는 달리 큰단위를 한번에 지울수있는 ROM, 즉 비휘발성 메모리이다.

Flash Memory에는 NOR 플래시 메모리와 NAND 플래시 메모리 두 종류가 있는데 병렬방식으로 동작하고 소비전력이 작은 NOR에비해 직렬구조로되어있지만 속도가 빠른 NAND 기반의 Flash는 많은 제조사들이 채택하고 있는 방식이다. 본 글에선 소자에 관련한 구체적인 동작원리에 대해서는 다루지 않는다.



Flash 칩은 결국 binary 비트를 저장할수있는 저장매체이다. 플래쉬를 구성하는 셀의 종류로는 단일 비트에대한 처리가 가능한 **single-level cell(SLC)** 및 두개의 비트 처리를 위한 **multi level cell(MLC)** 및  세개의 비트를 인코딩할수있는 **triple-level cell(TLC)** 까지 존재한다.

이중에는 **SLC** 가 성능이 가장 좋으면서도 비싸다.



---



### 2. Basic Flash Operations



플래쉬칩은 많은 셀들을 포함하는 bank 혹은 plane으로 이루어져있다.

이러한 bank를 구성하는 두가지 사이즈 단위로써 **Block**과 **Page**가 존재한다. 대게 블록은 128 or 256KB의 사이즈를 가지는 반면 페이지는 4KB정도의 크기를 가지는 정도이다.

이러한 Block과 Page로 구성된 플래쉬에는 크게 3가지 operation이 존재한다.



- ***Read(페이지단위)*** : client of the flash(컴퓨터의 file system등이 되겠다)는 Flash에 들어있는 데이터를 접근하기위하여 페이지 단위의 접근을 한다. 이때는 Random access의 특성을 가지며 모든 위치에 임의의 접근을 할 수있다.



- ***Erase(블럭단위)*** : 플래쉬에 데이터를 쓰기위해서는 반드시 Erase operation이 수반되어야 한다. Erase는 Block단위로 이루어지며 이는 플래쉬가 가지는 빠른 지우기특성이다. 하지만 플래쉬에 기록은 페이지단위로 이루어지기에, Erase 이전의 data copy 등의 이슈는 고려해야할 사항이다. 이러한 플래쉬의 Erase Operation은 몇 ms단위를 요구하는 가장 오버헤드가 긴 command 이다.



- ***Program(페이지단위)*** : 데이터를 Write하는 operation으로써 앞서나온 Erase Operation 이후 동작하는 operation이다. 반드시 Erase가 수반되어야 하며 이들를 묶어P/E(Program& Erase) 라고 부르기도 한다.

![image](https://user-images.githubusercontent.com/62167266/123730056-dc8da680-d8d0-11eb-847d-b2b60623ff13.png)

위는 플래쉬 operation의 일련의 동작과정을 나타내었다. Program 은 반드시 Erase를 거쳐 동작한다.



---



### 3. Flash Reliability



플래쉬의 신뢰성은 앞으로 다룰 가장 중요한 토픽이다.



- ***wear-out***

플래쉬 메모리의 특성상 P/E 를 거칠수록 전하가 축적되어 0과1을 구분할수없는 상태가 되는, wear-out이 일어난다. 따라서  모든 플래쉬 내 셀들을 균등하게 분산하여 메모리를 사용하도록 하는 `wear-leveling` 기법을 사용하며 이는 FTL의 핵심 부분이다.





- ***program disturbance***

또다른 신뢰성 문제는 disturbance 이다. 이는 특정 페이지를 참조할때 이웃페이지에도 bit가 flip되어 영향을 미칠수 있다는것이다.  FTL은 일반적으로 순차적인 write를 통해, 가령 낮은 페이지에서 높은 페이지의 순서로 지워진 블록의 페이지를 프로그래밍 함으로써 이러한 disturbance를 줄일수 있다.



---



### 4. Flash-Based SSD





![image](https://user-images.githubusercontent.com/62167266/123582340-95dc7580-d818-11eb-8192-ad226e0696a9.png)



위의 내용을 바탕으로 SSD와 같은  storage device에 대해 다루어보자.

SSD는 한마디로 Flash의 집합이라 할 수 있다.따라서 위에서다룬 신뢰성의 문제등을 sw레벨에서 control하고 device들을 orchestrate해주기 위한 control logic 이필요한데, 이를 **Flash Translation Layer(FTL)**이 담당한다.



(FTL의 구성 계층)

<img src="https://user-images.githubusercontent.com/62167266/123829538-83a82780-d93d-11eb-9858-3579547b5b86.png" alt="image" style="zoom:67%;" />



FTL은 Logical Block을 통해 처리된 데이터 읽기 쓰기등의 명령어들을 Physical block과 Physical page들에 대한 command로 바꾼다. 이과정에서 높은 성능 및 고신뢰성이 요구된다.

<img src="https://user-images.githubusercontent.com/62167266/123889692-fd1d3580-d990-11eb-8af3-fdb88bc18115.png" alt="image" style="zoom: 67%;" />





신뢰성 문제는 역시나 위에서 다룬 *wear-out* 및  *disturbance*를 해결하는것이고, 높은 성능을 위해 ***write amplication***에 대한 문제또한 해결해야한다.

**write amplication**이란 가령 Flash는 페이지단위로, 단 하나의 바이트만 기록하더라도 페이지단위로 기록하여야하고,  Erase 시에도 Block단위를 통해 erase 할 수있기 때문에 오버헤드가 발생한다. 

이에대한 다양한 해결법을 통해 FTL은 시스템 성능을 향상시킨다.



---



### 5. Naive idea for FTL  (Direct mapped FTL)



FTL을 구현하는 가장 단순한 알고리즘은 **Direct mapped**, 즉 직접매핑이다. FTL은 가상의 논리적 섹터를 통해 물리적페이지에 대응되는 논리적 페이지공간을 생성하는데, 이때 직접매핑에서는 물리적페이지 N은 논리적페이지 N에 대응된다.

이러한 접근은 메모리에 program을 하기위해 erase과정을 거쳐야하는 플래쉬의 특성때문에 물리페이지 N에대한 수정이나 쓰기가 다시 이루어질경우 반드시 해당페이지가 속한 Block을 모두 지우고 써야하므로 오버헤드가 크다. 이러한 접근법은 하드디스크보다 느린속도를 초래한다.



---

### 6. Log-Structured FTL



OS의  Log-Structured File System에서는 충분한 데이터가 모일때까지 버퍼링(buffering)후, 모아진 segment 별로 메모리에 write한다. 이때 무조건 새로운 위치에 write동작이 이루어지기때문에 물리적주소가 논리적 주소에 직접 대응되는 direct mapping 방식보다 효율적이라 할 수 있다.

 Log-Structured FTL의 경우에도 마찬가지로 새로운 데이터에 대해 기존의 데이터 다음공간에 write하게된다. 이를 logging 이라 한다.

또한 direct mapping과 다르게 논리적 주소는 고정된 물리적 메모리주소에 대응되지 않기때문에,  이를 위한 mapping table 을 가진다. 이는 flash에 위치해 있지만 SSD의 전원이 인가되면 빠른접근을 위해 RAM에 로드된다.



![image](https://user-images.githubusercontent.com/62167266/123586165-49486880-d81f-11eb-8932-5f5ea9bbe0be.png)



위와 같은 초기 상태를 가지는 SSD에서 다음의 명령어들을 수행한다고 가정해보자.

- Write(100) with contents a1

- Write(101) with contents a2

- Write(2000) with contents b1

- Write(2001) with contents b2



Flash의 특성상 초기  Invalid 블록에 대해 ERASE과정을 거쳐야 한다.

![image](https://user-images.githubusercontent.com/62167266/123586588-e7d4c980-d81f-11eb-8b0e-71018cd3d372.png)





이때 위의 write 대상 주소는 논리적 주소로써,물리 주소로 다음과 같이 매핑되었다고 가정하자.

이러한 매핑 테이블 (mapping table) 이슈 에 대해서도 뒤에서 다루어보도록 하자.

![image](https://user-images.githubusercontent.com/62167266/123586632-fd49f380-d81f-11eb-9208-a0cab06b9017.png)





그렇다면 여기서 논리메모리 100과 101 에 대한 데이터를 수정해준다고 가정해보자. 그렇다면 log-structured의 특성에 따라 새로운 location에 데이터를 넣게되고 아래와 같이 다른 physical address에 대응되게된다.

![image](https://user-images.githubusercontent.com/62167266/123586919-6c274c80-d820-11eb-9966-0c4af2e674ee.png)



이때 과거의 0번과 1번 page에 있는 데이터는 쓸모없는 Invalid 데이터이다. 따라서 아래에서 설명할 **Garbage Collection**의 수행과정을 거친다.



---



### 7. Garbage Collection



결론적으로 새로운 데이터가 write되면 쓸모없는 garbage blocks(or dead blocks)에 대해 메모리에서 삭제해주는 작업을 거쳐야 한다. (다음 write를 위해) 이를 **Garbage Collection** 이라고 한다.

다만 Flash에서의 erase는 block단위로 이루어지기에, block 내에는 garbage page를 포함하여 non-garbage page가 존재한다. 이때 non-garbage page에 대해서는 rewrite해주는 과정을 거친후, 해당 block을 erase한다.



![image](https://user-images.githubusercontent.com/62167266/123588339-91b55580-d822-11eb-8c5d-91adb2523c9a.png)

위에서는 기존의 100번지와 101번지에 대응되는 0번 및 1번페이지의 a1, a2 데이터가 garbage 데이터였다.

따라서 non-garbage 데이터인 b1 및 b2는 rewrite 해주고, 해당 블럭을 Erase 시킨다.

이러한 gc(garbage collection) 과정을 위해서는 mapping table 을 미리 체크해 기존의 블럭내에있던 페이지가 유지되는지 아닌지에대한 live information(현재 매핑정보) 을 사용하여 준비하는 절차를 거친다.  (가령 100과 101은 0번블럭에서 1번블럭으로 변경되었다. 따라서 gc 대상이다.)



이상적인 경우는 하나의 블록이 모두 garbage 페이지로 구성되어 그냥 지워버리는 경우일것이다. 

하지만 무한대의 physical memory가 존재하지는 않기때문에 때에 따라 non-garbage page가 존재하는 block을 erase하게된다. 이때는 non-garbage page에 대해 rewrite해주는 작업이 필요하다.



또한 gc 동작시 메모리 확보를 위해 즉각적인 erase를 시켜준다면 이또한 시스템 성능저하를 일으킨다. (erase는 실행동작시간이 긴 operation이다.)  따라서 garbage 데이터에 대해서 invalid( 혹은 stale) 상태를 부여하고 추후 백그라운드에서 gc동작을 여유롭게 주기적으로 수행한다.



아래설명할 두가지 기술들은 gc동작에 조금더 효율적인 방안을 제시한다.



**Over Provision**

SSD는  gc 비용을 줄이기 위해 가상의 파티션 공간을 추가하는 over provision 하는 기법을 채택한다.

이는 SSD 컨트롤러 레벨에서는 볼수있지만 파일시스템과같은 client측에서는 보이지않는 부분이다.

항상 언급되는 부분이지만 flash의 erase operation은 write보다 실행속도가 느리다. 따라서 gc를 통한 erase가 되기도 전에 모든 space가 소진되는 최악의 경우를 가정해 본다면, 이에 대한 해결방안이 필수적이다.

따라서 overprovision을 통한 공간을 통하여  write에대한 workload를 감당하는 일종의 버퍼로 사용하는 것이다.





**Trim**

단순히 말하자면 OS level에서 파일을 지우면 실제 SSD내부에서 데이터를 실제로 지우진 않는다. 왜냐하면 write 이상으로 erase는 실행시간에 오버헤드가 존재한다. 따라서 Logical 과 Physical을 잇는 metadata만 삭제하여 논리적으로만 지우게 되는셈이다.

하지만 gc의 특성상 invalid(stale)한 데이터에 대해 바로 삭제를 하지는 않는다. 따라서 지연된 삭제로인한 문제를 해결하고자 하는것이 **Trim**이다.

Trim명령은 OS수준에서 지원하며 지워진 데이터에대한 삭제를 명령하여 gc를 동작하도록 한다.





---

### 8. Mapping Table Size



Mapping Table 의 Size또한 고려해야할 요소이다.

4KB의 페이지를 가진 1TB의 SSD 를 고려할때, 대략 1TB에서 4KB로 나누면 1GB정도의 메모리가 필요하다. 한마디로 메모리 매핑에만 1GB가 필요하다.

따라서 page-level FTL은 메모리 측면에서 실용적이진 않다.



#### 8-1. Block-Based Mapping



![image](https://user-images.githubusercontent.com/62167266/123591215-a693e800-d826-11eb-9962-379ebf6b8f99.png)

위의 페이지 레벨 매핑을 대신하여 블록 레벨(Block-Level) 매핑이다. 한마디로 위에서 다루었던 매핑테이블에서 페이지단위가 아닌 위 그림과 같이 offset을 통해 블록단위로 물리 섹터를 논리섹터로 대응시킨다.

여기서 논리 메모리 주소인 2000, 2001, 2002, 2003 는 모두 같은 chunk인 500을 가진다고 가정하자.  이들은 모두 물리적 메모리인 1번블록의 4번페이지를 offset으로 저장되어있다.

따라서 논리주소 2001을 읽을시에는 매핑테이블의 4번페이지에 본인의 위치 기준 offset을 더하여 read할 수 있다.



하지만 새로운 데이터를 write 할시에는 오버헤드가 커지는데, 가령 2002 (현재 c 저장) 주소에 c' 데이터를 write한다고 가정할때, 500 chunk 에 있는 모든 데이터를 새로운 위치에 rewrite해야하고 기존의 블럭은 Erase시킨다.



![image](https://user-images.githubusercontent.com/62167266/123593001-f2e02780-d828-11eb-9763-dcea6a670cba.png)



이러한 블록 매핑의 특징으로는 결국 페이지 단위 매핑에서의 메모리 매핑사이즈를 줄이고자함인데, 블록단위의 포인터만 가지기에 이는 반대로 작은단위의 small한 write에 대해 문제를 발생시킨다.

가령 하나의 수정된 페이지의 write에 대해서도 블록내 모든 페이지들을 바꾸어주어야하는 문제점이  생긴다는 것이다.



#### 8-2. Hybrid Mapping



앞서 살펴본 block mapping 은 매핑 테이블의 사이즈는 줄일수 있지만 추가 작업의 오버헤드 때문에, 많은 현대의 FTL알고리즘은 페이지 레벨의 접근과 블록 레벨의 접근을 모두 가능하게하는 **Hybrid mapping** 을 채택한다.

Hybrid Mapping 에서는 페이지단위 매핑인 log table 과 블럭단위의 매핑인 data table 의 두가지 타입의 개념을 도입한다. 



![image](https://user-images.githubusercontent.com/62167266/123602447-b534cc00-d833-11eb-828c-5e18215a6932.png)

위의 그림에서 물리주소인 8~11 페이지에 각각 1000~1003 의 데이터인 a,b,c,d가 들어있다고 가정해보자. 이때는 Block-mapping과 동일하게 Data Table에 들어있는 하나의 블록 포인터(Block Pointer)로써 페이지 오프셋을 통해 데이터를 접근할 수 있다.



여기서 논리주소 1000~1003 즉 블럭내 모든 페이지의 데이터 수정이 일어났다고 가정해보자.

![image](https://user-images.githubusercontent.com/62167266/123603589-fe395000-d834-11eb-8e90-9a6e706e6a00.png)

페이지 수정에 대한 매핑은 Log Table을 통해 기록된다. 따라서 데이터를 찾기 위해서는 먼저 로그테이블을 참조해 최신업데이트된 데이터를 가져오고, 로그테이블에 데이터가 없다면 Data Table에 접근하여 데이터를  찾게된다.



중요한것은 log block수를 적게 유지하기위한 하이브리드 매핑전략의 특성에 있는데, 다시 Data Table을 통해 단일블록 포인터로만  가리킬수있는 블록으로 전환하게된다.

이때  로그블럭(Log Block)인 Block  '0'과 Block '2'와 동일한 방식으로 write되었기 때문에, switch merge 과정을 수행하게 된다. 

(여기서 말하는 동일한 방식이란, 해당블럭내의 페이지에 대한 데이터 수정을 의미한다.)



따라서 switch merge과정을 통해 해당 Log Block들을 merge하여 Free Block에 넣어주게 된다.

이러한 switch merge는 Hybrid FTL에서 best case라 할 수 있는데, 이러한 Log Block들을 merge하는 과정에서 모든 페이지에 대한 논리적 주소에따른 데이터가 변경되었기때문에 사실 데이터를 병합하는 과정은 필요가 없게 되는 것이다. (데이터 블록의 포인터만 바꾸어주면 된다.)



![image](https://user-images.githubusercontent.com/62167266/123605783-2cb82a80-d837-11eb-9fae-dc31e91bdfa9.png)



방금의 예에서는 블록내 모든 페이지에 대한 논리적주소에따른 데이터수정이 이루어졌다고 가정하였다. 하지만 블록내 페이지 전체가 교체되는것이 아닌,  아래와 같이 부분적인 데이터의 교환이 이루어질 수도 있다.

![image](https://user-images.githubusercontent.com/62167266/123606517-eca57780-d837-11eb-9880-8cdd18301407.png)



이때 FTL은 partial merge를 수행하게 되는데, 이는 위에서 언급한 switch merge의 결과와는 동일하지만, merge시 오버헤드가 증가하고 이에따른 `write amplication` 또한 증가한다.



마지막으로 full merge 또한 수행될 수 있는데, 위의 partial merge의 경우 다른 하나의 블록에 수정된 데이터에대해 merge해주었다. 반면 full merge의 경우 수정된 데이터가 여러 다른 log-block에 존재하여 merge의 오버헤드가 크다. 이는 시스템 성능에 큰 저하를 일으킨다.





#### 8-3. Page Mapping Plus Caching



하이브리드 매핑(Hybrid Mapping) 방식의 복잡성으로 인해 FTL의 활성 부분만 SSD의 메모리(RAM)에 캐시 하여 필요한 메모리의 양을 줄이는 방법이 제안되었다.

이는 결국 Page Mapping을 사용하되 단순한 페이지 매핑 방식은 많은 메모리 매핑 사이즈를 필요로 하기에 캐쉬 개념을 도입하다는 것이다.

하지만 이는 결국 필요한 매핑을 포함하고 있다면 좋은 성능을 보이지만, cache miss가 발생할경우 오버헤드가 증가한다.

하지만 대부분의 경우  지역성으로인한 캐시 접근 방식으로인해 메모리 오버헤드를 줄이고 성능을 높게 유지한다.



---



### 9. SSD Performance And Cost





![image](https://user-images.githubusercontent.com/62167266/123605398-c0d5c200-d836-11eb-9f49-2a6b5daccdc2.png)





---



### Reference 

https://pages.cs.wisc.edu/~remzi/OSTEP/

https://tech.kakao.com/2016/07/13/coding-for-ssd-part-1/

