[Part A]
first element, last element, random element, median of three를 pivot으로 하는 네가지 quick sort이다.

네가지 경우 모두 array를 partitioning하는 방법은 모두 동일하므로 sort함수를 만들었다.
이 함수는 실질적으로 quick sort를 구현하는데 arary의 왼쪽에서 출발하는 i와 오른쪽에서 출발하는 j를 이용한다.
이후 코드는 quick sort를 구현한 것이다.

random의 경우 10번을 반복해서 비교 횟수를 평균내는 방식이기 때문에
sorting 시작하기 전에 rbase라는 변수에 계속해서 base를 memcpy를 통해서 복사해 quick sort를 10번 반복하였다.

추가로 median of 3를 위해 medofthree함수를 만들었다.
첫번째, 중간, 마지막 원소를 비교하여 compar함수에 맞게 세 값을 sorting 하고 가운데 index를 return하였다.
이를 pivot으로 median of 3를 구현하였다.

네가지 경우 모두 pivot을 array의 가장 첫번째 원소로 옮겨 quick sort를 진행했다.

[Part B]
문제에서 원하는 것은 mean rating을 먼저 비교해서 크기가 큰 product가 우선순위를 가지고 동일하다면
salesVolume을 비교해서 우선순위를 정하는 것이다.
따라서 qsort 함수를 통해서 structure array인 products를 정렬하기 위해 compareProducts함수를 잘 정해야한다.
이 함수가 양수이면 qsort함수에서 swap하기 때문에 삼항 연산자를 통해서 함수값을 결정해주었다.

조건이 mean raitng이 우선 순위이므로 먼저 삼항 연산자를 통해 비교한다. 
A의 meanRating이 B의 meanRating보다 크다면 swap할 필요가 없기 때문에 -1로 return 한다.
다음으로 meanRating이 같은지도 확인하기 위해 위와 반대 조건으로 삼항 연산자를 연속해서 한 번 더 진행한다.
이미 첫번째 삼항 연산에서 A의 meanRating이 B것보다 작다는 조건이 생겼으므로 두번째 비교에서
B가 더 크지 않다는 결과가 나오면 이는 A와 B가 같다는 의미이다. 이 경우 salesVolume으로 비교를 해줘야한다.

위와 같이 compar함수를 만들면 원하는 결과를 얻을 수 있다.