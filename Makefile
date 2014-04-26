all: cantor2matrix gen_gamma_matrix capfilter gen_stratify Levenshtein_cantor_mpi gen_panchenko_features.cpp

Cantor2matrix: cantor2matrix.c
	gcc $^ -o $@
gen_gamma_matrix: gen_gamma_matrix.c
	gcc $^ -lm -o $@

capfilter: capfilter.cpp
	g++ $^ -lpcap -lm -o $@
gen_stratify: gen_stratify.cpp
	g++ $^ -o $@

Levenshtein_cantor_mpi: Levenshtein_cantor_mpi.cpp
	mpicxx $^ -o $@

gen_panchenko_features: gen_panchenko_features.cpp
	g++ $^ -lm -o $@

clean:
	rm -f ./*.o cantor2matrix gen_gamma_matrix capfilter gen_stratify Levenshtein_cantor_mpi gen_panchenko_features
