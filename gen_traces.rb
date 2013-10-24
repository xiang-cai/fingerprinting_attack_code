#!/usr/bin/env ruby


tornodes = ARGV[0] #"./tornodes_turtle_cold"
logfolder = ARGV[1] #"./log_turtle/"


1.upto(100) do |i|
	1.upto(10) do |j|
		`./capfilter /home/xcai/project/fingerprinting/httposcode_04_13/sshproxy/httposcaps/#{i}_#{j}.cap #{tornodes} 1 #{logfolder}#{i}_#{j}.txt`
	end

end


