#include "merge_sort.hpp"

static void merge(Record arr[], int left, int mid, int right) {
    int n1 = mid - left + 1;
    int n2 = right - mid;
    Record* L = new Record[n1];
    Record* R = new Record[n2];

    for (int i = 0; i < n1; ++i) L[i] = arr[left + i];
    for (int j = 0; j < n2; ++j) R[j] = arr[mid + 1 + j];

    int i = 0, j = 0, k = left;
    while (i < n1 && j < n2) {
        if (compositeCompare(L[i], R[j]))          arr[k++] = L[i++];
        else if (compositeCompare(R[j], L[i]))     arr[k++] = R[j++];
        else                                       arr[k++] = L[i++];
    }
    while (i < n1) arr[k++] = L[i++];
    while (j < n2) arr[k++] = R[j++];

    delete[] L;
    delete[] R;
}

void mergeSort(Record arr[], int left, int right) {
    if (left < right) {
        int mid = left + (right - left) / 2;
        mergeSort(arr, left, mid);
        mergeSort(arr, mid + 1, right);
        merge(arr, left, mid, right);
    }
}

bool compositeCompare(const Record& a, const Record& b) {
    return a.applicationNumber < b.applicationNumber;
}
