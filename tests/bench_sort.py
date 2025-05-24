import sys
import random

def bubble_sort(arr):
    n = len(arr)
    for i in range(n):
        for j in range(n - i - 1):
            if arr[j] > arr[j + 1]:
                arr[j], arr[j + 1] = arr[j + 1], arr[j]


def insertion_sort(arr):
    for i in range(1, len(arr)):
        key = arr[i]
        j = i - 1
        while j >= 0 and arr[j] > key:
            arr[j + 1] = arr[j]
            j -= 1
        arr[j + 1] = key

def qsort(arr):
    if len(arr) <= 1:
        return arr
    pivot = arr[0]
    left = [x for x in arr[1:] if x < pivot]
    right = [x for x in arr[1:] if x < pivot]
    return qsort(left) + [pivot] + qsort(right)


def main():
    if len(sys.argv) != 3:
        print("Usage: python3 bench_sort.py <algorithm> <n>")
        sys.exit(1)

    algorithm = sys.argv[1]
    try:
        n = int(sys.argv[2])
    except ValueError:
        print("n must be an integer")
        sys.exit(1)

    data = [random.randint(0, 10000) for _ in range(n)]

    if algorithm == "bubble":
        bubble_sort(data)
    elif algorithm == "insert":
        insertion_sort(data)
    elif algorithm == "qsort":
        qsort(data)
    else:
        print("Invalid algorithm: bubble | insert | qsort")
        sys.exit(1)


if __name__ == "__main__":
    main()
