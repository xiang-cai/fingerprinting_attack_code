#! /usr/bin/env ruby

#SVM customized kernel
	gamma = 2**0
	website = 100 
	while (website <= 100) do
		`rm ./gnorm_cus_t*`
		`./gen_stratify #{website} 0 ./tor_100_40_gnorm_matrix 1`

		trainsize = 36
		while (trainsize <= 36) do
			`rm ./gnorm_cus_training*`
			`./gen_stratify #{website} #{trainsize} ./tor_100_40_gnorm_matrix 0`

				cost = 2**2
				`echo 'gamma 2^0, cost 2^2, website #{website}, trainsize #{trainsize}' >> ./tor_result`
				1.upto(10) do |fold|
					`./svm-train -t 4 -c #{cost} ./gnorm_cus_training_#{fold} ./gnorm_cus_trainmodel >> ./tor_result`
					`./svm-predict ./gnorm_cus_testing_#{fold} ./gnorm_cus_trainmodel ./gnorm_cus_out >> ./tor_result`
					puts "website #{website} trainsize #{trainsize} fold #{fold} finished"
				end
				`echo '*********' >> ./tor_result`

		trainsize += 4
		end
	
	website += 50
	end
