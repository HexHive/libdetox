sub repeatloop {
  local($loops) = @_;
  for ($i = 0; $i < $loops; $i++) {
  }
}

sub ifact1loop {
  local($loops) = @_;
  for ($i = 0; $i < $loops; $i++) {
    $value = 1;
    for ($j = 1; $j <= 100; $j++) {
      $value *= $j;
    }
  }
}

sub ifact2loop {
  local($loops) = @_;
  for ($i = 0; $i < $loops; $i++) {
    for ($j = 1; $j <= 100; $j++) {
      if ($j <= 1) {
	$value = 1;
      }
      else {
	$value *= $j;
      }
    }
  }
}

sub fact {
  local($value) = @_;
  if ($value <= 1) {
    $value = 1;
  }
  else {
    $value *= &fact($value - 1);
  }
}

sub factloop {
  local($loops) = @_;
  for ($i = 0; $i < $loops; $i++) {
    &fact(100);
  }
}

sub stemsloop {
  local($loops) = @_;
  for ($i = 0; $i < $loops; $i++) {
    open(F, "words");
    read(F, $raw, 1000000); # read all of it
    close(F);
    @words = split(/\s+/, $raw);
    foreach $w (@words) {
      if (length($w) == 4) {
        $lastfour = $w;
      }
      elsif (length($w) == 5 && index($w, "'") == -1
  	    && ((substr($w, 4, 1) ne "s"
  		 || substr($w, 0, 4) ne $lastfour))) {
        $wordlist{substr($w, 0, 3)} .= $w;
      }
    }
    foreach $w (sort keys(%wordlist)) {
      $t = $wordlist{$w};
      $nwords = length($t) / 5;
      if ($nwords > 1) {
        $outputstring .= "$w $nwords\t" . substr($t, 0, 5) . "\n";
        for ($n = 1 ; $n < $nwords ; $n++) {
  	$outputstring .= "\t" . substr($t, $n * 5, 5) . "\n";
        }
      }
    }
  }
#  print $outputstring;
}

sub systemloop {
  local($loops) = @_;
  for ($i = 0; $i < $loops; $i++) {
    system("echo test >/dev/null");
  }
}

sub fileloop {
  local($loops) = @_;
  for ($i = 0; $i < $loops; $i++) {
    $j = 0;
    open(MYOUTPUT,">/tmp/tmp");
    while ( $j < 100 ) {
      print MYOUTPUT "PRL LINE -> $j\n";
      $j = $j + 1;
    }
    close (MYOUTPUT);
    open(MYOUTPUT,"/tmp/tmp");
    $j = 0;
    while ( <MYOUTPUT> ) {
      $j = $j + 1;
    }
    close (MYOUTPUT);
    if ($j ne 100) {
      print "WARNING: Retrieved only $j lines!\n";
    }
  }
}

$NREPEATS = 1000000;
$NIFACT1 = 10000;
$NIFACT2 = 10000;
$NFACT = 10000;
$NSYSTEM = 1000;
$NFILE = 2000;
$NSTEMS = 10;


#$NREPEATS = 10;
#$NIFACT1 = 10;
#$NIFACT2 = 10;
#$NFACT = 10;
#$NSYSTEM = 10;
#$NFILE = 20;
#$NSTEMS = 10;


$overall = 0;

$before = time();
&repeatloop($NREPEATS);
$after = time();
$total = $after - $before;
print "$NREPEATS repeats in $total seconds\n";
$overall = $overall + $total;

$before = time();
&ifact1loop($NIFACT1);
$after = time();
$total = $after - $before;
print "$NIFACT1 iterative fact(100) in $total seconds\n";
$overall = $overall + $total;

$before = time();
&ifact2loop($NIFACT1);
$after = time();
$total = $after - $before;
print "$NIFACT2 iterative fact(100) with 'if' in $total seconds\n";
$overall = $overall + $total;

$before = time();
&factloop($NFACT);
$after = time();
$total = $after - $before;
print "$NFACT recursive factorial(100) in $total seconds\n";
$overall = $overall + $total;

$before = time();
&stemsloop($NSTEMS);
$after = time();
$total = $after - $before;
print "$NSTEMS stems generation in $total seconds\n";
$overall = $overall + $total;

$before = time();
&systemloop($NSYSTEM);
$after = time();
$total = $after - $before;
$overall = $overall + $total;
print"$NSYSTEM system calls echo test in $total seconds\n";

$before = time();
&fileloop($NFILE);
$after = time();
$total = $after - $before;
$overall = $overall + $total;
print"$NFILE 100-line file reads and writes in $total seconds\n";

print"Overall test executed in $overall seconds\n";
