#include <vector>

//merge function
template <typename T>
void merge(std::vector<T>& arr, int left, int mid, int right) {
    int n1 = mid - left + 1;
    int n2 = right - mid;

    std::vector<T> L(n1), R(n2);
    for (int i = 0; i < n1; i++) L[i] = arr[left + i];
    for (int j = 0; j < n2; j++) R[j] = arr[mid + 1 + j];

    int i = 0, j = 0, k = left;
    while (i < n1 && j < n2) {

        if (!(R[j] < L[i]))  
            arr[k++] = L[i++];
        else
            arr[k++] = R[j++];
    }

    while (i < n1) arr[k++] = L[i++];
    while (j < n2) arr[k++] = R[j++];
}

// sorting algorithm
template <typename T>
void sort(std::vector<T>& arr, int left, int right) {
    if (left >= right) return;

    int mid = left + (right - left) / 2;
	sort(arr, left, mid);
	sort(arr, mid + 1, right);
	merge(arr, left, mid, right);
}

//helper overload for easier function call
template <typename T>
void sort(std::vector<T>& arr) {
    if (!arr.empty())
	sort(arr, 0, arr.size() - 1);
}
