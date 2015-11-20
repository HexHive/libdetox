#!/usr/bin/ruby

require 'strscan' # for StringScanner

####################### CONFIGURATION

$PROJECTROOT = Dir.pwd + "/"
$RT = $PROJECTROOT + "bin/trustedRT"
$LIB = $PROJECTROOT + "lib/libtrustedRT.so.0.3.0"
$RTLD = $PROJECTROOT + "lib/trustedRTLD.so"
$TESTMODE = 0 # 0: run all tests, 1: native, 2: ldpreload, 3: runtime, 4: rtld, 5: only the ones including the loader

####################### CLASS DEFINITIONS

class TestCase

  attr_reader :status, :results, :exectimes

  def initialize(path)
    @name   = File.dirname(path) # path to the dir containing test executable
    @path   = path # path to main test executable
    @status = [0,0,0,0] # 0 = not run, -1 = run failed, 1 = run succeeded
    @results = [-1,-1,-1,-1] # store results here
    @exectimes = [0,0,0,0] # not used at the moment
    @expectedret = [0,0,0,0]
    @runconfig = [0,0,0,0]
    
    # read test configuration file
	text = File.open(File.dirname(@path)+"/TESTCONFIG").read
	text.each_line do |line|
		s = StringScanner.new(line)
		s.scan(/(\w+) (\d) (\d+)/)
		case s[1]
		when "NATIVERUN"
			if s[2] == "1"
				@runconfig[0] = 1
				@expectedret[0] = s[3]
			end
		when "LDPRELOADRUN"
			if s[2] == "1"
				@runconfig[1] = 1
				@expectedret[1] = s[3]
			end
		when "RUNTIMERUN"
			if s[2] == "1"
				@runconfig[2] = 1
				@expectedret[2] = s[3]
			end
		when "RTLDRUN"
			if s[2] == "1"
				@runconfig[3] = 1
				@expectedret[3] = s[3]
			end
		end
	end
	
  end
 
  # nativeRun method executes "main" without any translation
  def nativeRun
    if @runconfig[0] == 1
		print "\t\tnativeRun "
		
		# TODO: use popen3 and catch segfaults
		#require 'open3'
		#Open3.popen3(@path) do |stdin, stdout, stderr, wait_thr|
			#exit_status = wait_thr.value
			#@results[0] = exit_status.exitstatus
		#end
		
		#system(@path + " 1>/dev/null")
		Dir.chdir(File.dirname(@path)){
			#system("./main 1>/dev/null")
			system("./main > /dev/null 2>&1")
		}
		@results[0] = $?.exitstatus
		if @results[0] == @expectedret[0].to_i
			@status[0] = 1
			print "\t\t\t\t\tOK"
		else
			@status[0] = -1
			print "\t\t\t\t\tFAILED"
		end
		print " ("
		print @results[0]
		print ")"
		print "\n"
	end
  end
  
  # ldpreloadRun method executes "main" with LD_PRELOAD=libfastbt
  def ldpreloadRun
    if @runconfig[1] == 1
		print "\t\tldpreloadRun "
		#system("LD_PRELOAD=" + $LIB + " " + @path + " 1>/dev/null")
		Dir.chdir(File.dirname(@path)){
			#system("LD_PRELOAD=" + $LIB + " ./main 1>/dev/null")
			system("LD_PRELOAD=" + $LIB + " ./main > /dev/null 2>&1")
		}
		@results[1] = $?.exitstatus
		if @results[1].to_i == @expectedret[1].to_i
			@status[1] = 1
			print "\t\t\t\t\tOK"
		else
			@status[1] = -1
			print "\t\t\t\t\tFAILED"
		end
		print " ("
		print @results[1]
		print ")"
		print "\n"
	end
  end
  
  # rtRun method executes "main" within the trusted runtime executable
  def rtRun
    if @runconfig[2] == 1
		print "\t\truntimeRun "
		#system($RT + " " + @path + " 1>/dev/null")
		Dir.chdir(File.dirname(@path)){
			#system($RT + " ./main 1>/dev/null")
			system($RT + " ./main > /dev/null 2>&1")
		}
		@results[2] = $?.exitstatus
		if @results[2] == @expectedret[2].to_i
			@status[2] = 1
			print "\t\t\t\t\tOK"
		else
			@status[2] = -1
			print "\t\t\t\t\tFAILED"
		end
		print " ("
		print @results[2]
		print ")"
		print "\n"
	end
  end

  # rtldRun executes "main.rtld" if it exists, "main.rtld" is the same executable
  # like "main" just with our RTLD within the INTERP ELF program header
  def rtldRun
    if @runconfig[3] == 1
		print "\t\trtldRun "
		if File.exists?(@path+".rtld")
				#system(@path.+".rtld" + " 1>/dev/null")
				Dir.chdir(File.dirname(@path)){
					#system("./main.rtld 1>/dev/null")
					system("./main.rtld > /dev/null 2>&1")
				}
				@results[3] = $?.exitstatus
				if @results[3] == @expectedret[3].to_i
					@status[3] = 1
					print "\t\t\t\t\tOK"
				else
					@status[3] = -1
				print "\t\t\t\t\tFAILED"
				end
		else
			@results[3] == -1
			@status[3] = 0
			print "\t\t\t\t\tNORUN"
		end
		print " ("
		print @results[3]
		print ")"
		print "\n"
	end
  end
  
  def to_s
	"TestCase: #{@name}\n"
  end
  
  def runAll
	puts "\t#{@name}\n"
	self.nativeRun
	self.ldpreloadRun
	self.rtRun
	self.rtldRun
	puts "\n"
  end
  
  def runNative
	puts "\t#{@name}\n"
	self.nativeRun
	puts "\n"
  end
   
  def runLDPRELOAD
	puts "\t#{@name}\n"
	self.ldpreloadRun
	puts "\n"
  end
  
  def runRunTime
	puts "\t#{@name}\n"
	self.rtRun
	puts "\n"
  end
  
  def runRTLD
	puts "\t#{@name}\n"
	self.rtldRun
	puts "\n"
  end
  
  def runOnlyWithLoader
	puts "\t#{@name}\n"
	self.nativeRun
	self.rtRun
	self.rtldRun
	puts "\n"
  end
  
end

####################### MAIN

# print banner
puts "\n"
puts ">>>>>>>>>> TRuE Automated TestCase Execution <<<<<<<<<<\n"
puts "\n"

puts "CONFIGURATION"
puts "\n"
puts "ROOT:\t\t" + $PROJECTROOT
puts "LIB:\t\t" + $LIB
puts "RT:\t\t" + $RT
puts "RTLD:\t\t" + $RTLD
puts "\n"

# check commandline arguments for testmode switch
if ARGV.count != 0
 case ARGV[0]
 when "-all"
	$TESTMODE = 0
 when "-native"
	$TESTMODE = 1
 when "-ldpreload"
	$TESTMODE = 2
 when "-runtime"
	$TESTMODE = 3
 when "-rtld"
	$TESTMODE = 4
 when "-wloader"
	$TESTMODE = 5
 end
else
	puts "Usage: runtests.rb -{all,native,ldpreload,runtime,rtld,wloader}"
	puts
	exit
end

# create the array containing all the TestCase objects
testCases = Array.new

# find all **/main files and for each of them create a TestCase object
mainExecutables = File.join("**", "main")
Dir.glob(mainExecutables) {|x| testCases.push(TestCase.new(x)) }

puts "Nr of TestCases found: #{testCases.count}\n\n"
puts "Run TestCases...\n\n"

# initialize summary variables
totalNr = testCases.count

totalRuns = 0
totalSucceeded = 0
totalFailed = 0
totalNotRun = 0

puts "TestMode = #{$TESTMODE}\n\n"

# run testcases according to selected test mode
case $TESTMODE
 when 0
	# get current timestamp
	start = Time.now
	# run all tests
	testCases.each {|x| x.runAll }
	# calculate elapsed time
	execTime = Time.now - start
 when 1
	# get current timestamp
	start = Time.now
	# run all tests
	testCases.each {|x| x.runNative }
	# calculate elapsed time
	execTime = Time.now - start
 when 2
	# get current timestamp
	start = Time.now
	# run all tests
	testCases.each {|x| x.runLDPRELOAD }
	# calculate elapsed time
	execTime = Time.now - start
 when 3
	# get current timestamp
	start = Time.now
	# run all tests
	testCases.each {|x| x.runRunTime }
	# calculate elapsed time
	execTime = Time.now - start
 when 4
	# get current timestamp
	start = Time.now
	# run all tests
	testCases.each {|x| x.runRTLD }
	# calculate elapsed time
	execTime = Time.now - start
 when 5
	# get current timestamp
	start = Time.now
	# run all tests
	testCases.each {|x| x.runOnlyWithLoader }
	# calculate elapsed time
	execTime = Time.now - start
end

puts "Run TestCases... done!\n\n"

# get test case result numbers
testCases.each {|x| totalSucceeded += (x.status.grep(1).size) }
testCases.each {|x| totalFailed += (x.status.grep(-1).size) }
testCases.each {|x| totalNotRun += (x.status.grep(0).size) }

totalRuns = totalSucceeded + totalFailed

# print execution summary
puts ">> Execution Summary"
puts " ------------------------------"
puts " Total Nr. of TestCases: " + totalNr.to_s
puts " Total Nr. of TestCase Runs: " + totalRuns.to_s
puts " ------------------------------"
puts " Total OK: " + totalSucceeded.to_s + " / " + (totalSucceeded.to_f/totalRuns.to_f * 100).round.to_s + "%"
puts " Total FAILED: " + totalFailed.to_s + " / " + (totalFailed.to_f/totalRuns.to_f * 100).round.to_s + "%"
puts " Total Execution Time: " + execTime.to_s + "s"
puts ""
