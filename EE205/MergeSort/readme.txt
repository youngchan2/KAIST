[Part A]
 To implement the mergesort, I use two functions which are mergesort and merge.
First, the function 'mergesort' is used for recursive call. To divide the given arrays as two arrays,
I approach the array with the indices. Calculate the middle index(mid) by (start+end)/2.
Left array will start with 'start' index and end with 'mid' index. 
Right array will start with 'mid+1'index and end with 'end' index. 
After calling 'meregesort' fucntion for each array, call 'merge' function to merge these two arrays.
 To merge arrays, I also use the indices. Before merging the arrays, I make the temporay array for sorting.
Then, find the left array pointer and right array pointer with these indices.
Using the while loop, this code will merge left and right arrays with an order which will be followed by 'compar' function.

[Part B]
 For the first optimization, I decide the threshold as 16. If the number of the items is less than threshold, I apply the insertion sort instead of merge sort.
For the second optimization, I add the one condition before calling 'merge' function. The condition is 'compare the base[mid] and base[mid+1]'. Let's assume that I try to sort as an ascending order. 
If base[mid+1] is bigger than base[mid], it means that this array is already sorted.
 The result of the 'mergesort' funtion is the sorted array. Therefore if the last item of left array is smaller than the first item of right array, I don't have to call 'merge' fucntion.
