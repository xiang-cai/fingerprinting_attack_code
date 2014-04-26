#! /usr/bin/env ruby


# SVM with all features
gamma = 2**19
gamma = 1.0/gamma
cost = 2**17

`rm ./svm_results`

web = 220
while web <= 400 do
	`./svm-train -c #{cost} -g #{gamma} -v 10 modssh_CPSM_svm_fvector_#{web}_18 >> ./svm_results`	
	`echo 'modssh_CPSM_svm_fvector_#{web}_18' >> ./svm_results`
	`echo '=============' >> ./svm_results`
	puts "modssh_CPSM_svm_fvector_#{web}_18 done"
	web += 40
end




=begin
`rm ./svm_results`
19.upto(19) do |powg|
	gamma = 2**powg
	gamma = 1.0/gamma
	17.upto(17) do |powc|
		cost = 2**powc
		1.upto(1) do |fold|
			`echo 'gamma=2^#{powg} cost=2^#{powc}' >> ./svm_results`
			`./svm-train -c #{cost} -g #{gamma} -v 10 ./svm_fvector >> ./svm_results`
			`echo '=============' >> ./svm_results`
#			`./svm-train -g #{gamma} -c #{cost} ./svm_training_#{fold} ./trainmodel`
#			`./svm-predict ./svm_testing_#{fold} ./trainmodel ./svm_out >> ./svm_results`
		end
	end
end
=end

#SVM customized kernel
=begin
-5.upto(2) do |powg|
	gamma = 2**powg
	`rm ./gnorm*`
	`./gen_stratify ./minnorm_matrix_g#{powg}.dat`

	0.upto(5) do |powc|
		cost = 2**powc
		`echo 'gamma 2^#{powg}, cost 2^#{powc}' >> ./minnorm_stratify_results`
		1.upto(10) do |fold|
			`./svm-train -t 4 -c #{cost} ./gnorm_cus_training_#{fold} ./gnorm_cus_trainmodel`
			`./svm-predict ./gnorm_cus_testing_#{fold} ./gnorm_cus_trainmodel ./gnorm_cus_out >> ./minnorm_stratify_results`
		end
		`echo '*********' >> ./minnorm_stratify_results`
=begin
		1.upto(100) do |i|
			`./svm-train -t 4 -g #{gamma} -c #{cost} ../svm#{i}_trainkernel ../svm#{i}_trainmodel`
			`echo 'svm#{i} gamma 2^#{powg} cost 2^#{powc}' >> ../svm_largetestingrate_c17g19`
			`./svm-predict ../svm#{i}_largetestkernel ../svm#{i}_trainmodel ../svm#{i}_out >> ../svm_largetestingrate_c17g19`
			`echo ' ' >> ../svm_largetestingrate_c17g19`
		end
=end
=begin
	end
end
=end
