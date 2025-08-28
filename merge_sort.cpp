#include "merge_sort.hpp"

static void merge(OrderRecord arr[], int left, int mid, int right) {
    int n1 = mid - left + 1;
    int n2 = right - mid;
    OrderRecord* L = new OrderRecord[n1];
    OrderRecord* R = new OrderRecord[n2];

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

void mergeSort(OrderRecord arr[], int left, int right) {
    if (left < right) {
        int mid = left + (right - left) / 2;
        mergeSort(arr, left, mid);
        mergeSort(arr, mid + 1, right);
        merge(arr, left, mid, right);
    }
}

bool compositeCompare(const OrderRecord& a, const OrderRecord& b) {
    return a.cost < b.cost;
}
