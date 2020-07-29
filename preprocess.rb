#!/usr/bin/env ruby

def key_definition(hex_code, indent='  ')
	bytes = []
	count_ones = count_zeros = 0
	hex_code.scan(/../) do |byte|
		bytes << byte.hex
	end
	bits_str = bytes.map{|b| '%016b' % ((b<<8) + (255-b)) }.join()
	$stderr.puts " ...translated to #{bits_str}"
	sequence = ''
	bits_str.chars.each do |b|
		case b
		when ?0
			count_zeros += 1
			sequence += 'zero();'
		when ?1
			count_ones += 1
			sequence += 'one();'
		end
	end
	delay = "_delay_us(REPEAT_TIME - #{count_zeros}*ZERO_TIME - #{count_ones}*ONE_TIME);"
	"\n#{indent}#{sequence}\n#{indent}#{delay}\n"
end

ARGF.each do |line|
	puts line.gsub(/_key_definition\((.*?)\)/) {|m|
		case $1
		when 'x'
			m
		when /0x(([0-9a-f][0-9a-f])+)/
			$stderr.puts "Found definition for key with code #{$1}"
			key_definition($1, '    ')
		else
			raise "_key_definition failed for \"#{m}\""
		end
	}
end
