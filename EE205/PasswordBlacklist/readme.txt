(1)
hash map을 이용해 약 25만개의 입력값을 관리하였다. string hash function은 djb2 알고리즘을 사용해 hash code를 구한다. Node structure는 password string과 횟수를 저장한다. 추가로 index 충돌을 방지하기 위한 separate chaining을 구현했다. 따라서 다음 node를 알려줄 next도 포함한다.
password structure는 node의 structure array로 구현한 hash map을 가지고 있고 curcnt 변수를 통해 hash map이 어느 정도 차있는지 저장하였다. fgets함수를 통해서 password를 입력받으면 insert함수를 통해서 hash map으로 mapping되는데 이 때 hash map에서 password가 있으면
해당 structure node를 strcmp 함수를 통해 찾아 횟수를 update해준다. node가 없다면 hash map[index]에 새 node를 추가해준다. (fgets는 '\n'도 포함시켜 이 부분을 '\0'으로 바꿨다.) 이 때 resize함수를 통해서 hash map의 load된 정도가 load factor를 넘어서면 두 배로 hash map의 크기를 늘려주었다.
결과 출력을 위해 print함수로 이동하면 qsort를 진행하기 위해 allnodes함수를 호출한다. allnodes 함수는 hashmap에 있는 모든 linked list의 node를 꺼내 하나의 array로 합치는 과정이다.
node의 num을 기준으로 정렬한 compare함수를 통해서 qsort를 진행한 후 명령어의 argument로 받은 숫자만큼 password를 출력한다. 마지막으로 freeall함수를 통해서 hashmap의 메모리를 free시켜주었다.

extra credit 부분은 명령어의 argument로 구분하였다. main함수의 argument인 argc가 3이면 extra credit에 해당하는 부분이다. password를 입력받은 후 명령어의 세번째 인자(argv[2])를 문자열의 길이로 사용한다. 
strncpy함수를 통해 입력받은 password의 substring을 만들어 insert 함수의 인자로 사용했다.
argc가 2인 경우 password는 별도의 처리 없이 바로 insert함수의 인자로 사용했다.

이 알고리즘의 worst case는 hash map의 한칸에 모두 mapping 되는 경우로 O(n) linear time이 걸린다.

(2)
주석처리 된 부분을 통해 정확성을 확인했다.
blacklist를 ./blacklist < ./leakedPasswordList.txt > a.txt 명령어로 실행시켜 a.txt에 184389개의 password가 출력됨을 확인했다.
명령어 sort ./ leakedPasswordList.txt | uniq -c | sort -k 1 -r -n | more를 통해 나온 결과를 b.txt파일로 만든 후
주석처리 된 check함수를 통해 b.txt파일을 한줄씩 읽어 password를 blacklist 안에서 만든 hash map에서 찾는 과정을 거쳤다.
이 때 입력받은 password와 횟수를 hash map과 비교했을 때 동일하면 ans를 1씩 더해주었는데 최종 결과가 184389로 모두 동일함을 확인했다.
따라서 만든 blacklist 프로그램은 정확하다.

(3)
./blacklist 5 명령어를 입력했을 경우 0.128초가 걸렸고 ./blacklist 5 5 명령어를 입력했을 경우 0.326초가 걸렸다.
time.h library안의 clock함수를 이용해 프로그램 실행 시간을 확인했다.