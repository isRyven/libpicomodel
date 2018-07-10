template<int n, typename T>
bool comparevec(T a, T b) {
	for (int i = 0; i < n; i++) if (a[i] != b[i]) return false;
	return true;
}

template<int n, typename T>
bool comparevec_approx(T a, T b, float epsilon) {
	for (int i = 0; i < n; i++) if (std::fabs(a[i] - b[i]) > epsilon) return false;
	return true;
}
