#!/usr/bin/perl

select (STDERR) ;
$| = 1 ;
select (STDOUT) ;
$| = 1 ;
use strict ;

############# Define some constants ##############
use constant FAIL     => 0 ;
use constant PASS     => 1 ;
use constant NOT_RUN  => 2 ;
use constant GET_AREA     => 11 ;
use constant GET_TIMING   => 12 ;
use constant GET_ERROR    => 13 ;
##################################################

### Global Variables #### --we must try to reduce these
my $run_directory = $ARGV[0]."/" ;
my $SYNPLIFY_SYNTHESIS=0 ;
my $technology="xilinx_virtex5" ;
my $logic_synthesis_only = 0 ;
my %cost_str_map = ();
my %cost_display_map = ();
my $extract_cost_postfix = "" ;
my $timing_slack_search_str ;
my $timing_score_search_str ;
my $num_luts="" ;
my $num_ffs="" ;
my $num_dsps="" ;
my $num_brams="" ;

my $is_combinational_only=0 ;
my $timing_slack=-1 ;
my $timing_score="" ;

my $logic_synth_file=""  ;
my $ngdbuild_file="" ;
my $map_file="" ;
my $par_file="" ;

my $logic_synth_status="" ;
my $ngdbuild_status="" ;
my $map_status="" ;
my $par_status="" ;

my $logic_synth_file_exists ;
my $ngdbuild_file_exists ;
my $map_file_exists ;
my $par_file_exists ;

my $timing_status ;
my $run_flow_status ;

####### message statements can be changes here accordingly change the alignment number currently 16 #####
my $spacing=16;
my $logic_synth_message= ($SYNPLIFY_SYNTHESIS==1) ? "synplify_pro" : "xst" ;
my $ngdbuild_message="ngdbuild" ;
my $map_message="map" ;
my $par_message="par" ;
my $area_message="Area" ;
my $slack_message="Slack" ;
my $timing_score_message="Timing Score" ;
my $timing_score_implication="" ;


my %xilinx_spartan_3a_dsp_cost_str_map = (
    'LUT'   => 'Total Number of 4 input LUTs',
    'FF'    => 'Number of Slice Flip Flops',
    'BRAM'  => 'Number of RAMB16.*',
    'DSP'   => 'Number of DSP48As',
);

my %xilinx_spartan_3a_dsp_cost_display_map = (
    'LUT'  => "LUTs",
    'FF'   => "FFs" ,
    'BRAM' => "BRAMs",
    'DSP'  => "DSPs"
);

my %xilinx_spartan3e_cost_str_map = (
    'LUT'   => 'Total Number of 4 input LUTs',
    'FF'    => 'Number of Slice Flip Flops',
    'BRAM'  => 'Number of .*RAM.*',
    'DSP'   => 'Number of MULT18X18.*',
);

my %xilinx_spartan3e_cost_display_map = (
    'LUT'   => "LUTs",
    'FF'    => "FFs" ,
    'BRAM'  => "BRAMs",
    'DSP'   => "MULTs"
);

my %xilinx_virtex4_cost_str_map = (
    'LUT'   => 'Total Number of 4 input LUTs',
    'FF'    => 'Number of Slice Flip Flops',
    'BRAM'  => 'Number of FIFO16/RAMB16s',
    'DSP'   => 'Number of DSP48s',
);

my %xilinx_virtex5_cost_str_map = (
    'LUT'   => 'Number of Slice LUTs',
    'FF'    => 'Number of Slice Registers',
    'BRAM'  => 'Number of Block\s*RAM/FIFO',
    'DSP'   => 'Number of DSP48.*',
);

my %xilinx_spartan6_cost_str_map = (
    'LUT'   => 'Number of Slice LUTs',
    'FF'    => 'Number of Slice Registers',
    'BRAM'  => 'Number of .*RAM.*',
    'DSP'   => 'Number of DSP48.*',
);

my %xilinx_spartan6l_cost_str_map = (
    'LUT'   => 'Number of Slice LUTs',
    'FF'    => 'Number of Slice Registers',
    'BRAM'  => 'Number of .*RAM.*',
    'DSP'   => 'Number of DSP48.*',
);

my %xilinx_virtex6_cost_str_map = (
    'LUT'   => 'Number of Slice LUTs',
    'FF'    => 'Number of Slice Registers',
    'BRAM'  => 'Number of .*RAM.*',
    'DSP'   => 'Number of DSP48.*',
);

my %xilinx_virtex6l_cost_str_map = (
    'LUT'   => 'Number of Slice LUTs',
    'FF'    => 'Number of Slice Registers',
    'BRAM'  => 'Number of .*RAM.*',
    'DSP'   => 'Number of DSP48.*',
);

my %synplicity_xilinx_spartan3e_cost_str_map = (
    'LUT'   => 'Total\s*LUTs:\s*(\S+)\s*\(\S+%\)',
    'FF'    => 'Register bits not including I/Os:\s*(\S+)\s*\(\S+%\)',
    'BRAM'  => 'Block Rams\s*:\s*(\S+)\s*of\s*\S+\s*\(\S+%\)',
    'DSP'   => 'MULT18X18SIO\s*(\S+)\s*uses',
);

my %synplicity_xilinx_spartan_3a_dsp_cost_str_map = (
    'LUT'   => 'Total\s*LUTs:\s*(\S+)\s*\(\S+%\)',
    'FF'    => 'Register bits not including I/Os:\s*(\S+)\s*\(\S+%\)',
    'BRAM'  => 'Block Rams\s*:\s*(\S+)\s*of\s*\S+\s*\(\S+%\)',
    'DSP'   => 'DSP48s:\s*(\S+)\s*of\s*\S+\s*\(\S+%\)',
);

my %synplicity_xilinx_spartan6_cost_str_map = (
    'LUT'   => 'Total\s*LUTs:\s*(\S+)\s*\(\S+%\)',
    'FF'    => 'Register bits not including I/Os:\s*(\S+)\s*\(\S+%\)',
    'BRAM'  => 'RAMB\S*\s*(\S+)\s*uses',
    'DSP'   => 'DSP48s:\s*(\S+)\s*of\s*\S+\s*\(\S+%\)',
);

my %synplicity_xilinx_spartan6l_cost_str_map = (
    'LUT'   => 'Total\s*LUTs:\s*(\S+)\s*\(\S+%\)',
    'FF'    => 'Register bits not including I/Os:\s*(\S+)\s*\(\S+%\)',
    'BRAM'  => 'RAMB\S*\s*(\S+)\s*uses',
    'DSP'   => 'DSP48s:\s*(\S+)\s*of\s*\S+\s*\(\S+%\)',
);

my %synplicity_xilinx_virtex6_cost_str_map = (
    'LUT'   => 'Total\s*LUTs:\s*(\S+)\s*\(\S+%\)',
    'FF'    => 'Register bits not including I/Os:\s*(\S+)\s*\(\S+%\)',
    'BRAM'  => 'RAMB\S*\s*(\S+)\s*uses',
    'DSP'   => 'DSP48s:\s*(\S+)\s*of\s*\S+\s*\(\S+%\)',
);

my %synplicity_xilinx_virtex6l_cost_str_map = (
    'LUT'   => 'Total\s*LUTs:\s*(\S+)\s*\(\S+%\)',
    'FF'    => 'Register bits not including I/Os:\s*(\S+)\s*\(\S+%\)',
    'BRAM'  => 'RAMB\S*\s*(\S+)\s*uses',
    'DSP'   => 'DSP48s:\s*(\S+)\s*of\s*\S+\s*\(\S+%\)',
);

my %xst_xilinx_spartan_3a_dsp_cost_str_map = (
    'LUT'   => 'Number of 4 input LUTs',
    'FF'    => 'Number of Slice Flip Flops',
    'BRAM'  => 'Number of BRAMs',
    'DSP'   => 'Number of DSP48s',
);

my %xst_xilinx_spartan3e_cost_str_map = (
    'LUT'   => 'Number of 4 input LUTs',
    'FF'    => 'Number of Slice Flip Flops',
    'BRAM'  => 'Number of .*RAM.*',
    'DSP'   => 'Number of MULT18X18.*',
);

my %xst_xilinx_virtex4_cost_str_map = (
    'LUT'   => 'Number of 4 input LUTs',
    'FF'    => 'Number of Slice Flip Flops',
    'BRAM'  => 'Number of FIFO16/RAMB16s',
    'DSP'   => 'Number of DSP48s',
);

sub init_cost_info
{
	if($logic_synthesis_only)  {
		if($SYNPLIFY_SYNTHESIS) {
			$extract_cost_postfix='' ;
			$cost_str_map{xilinx_spartan_3a_dsp} = \%synplicity_xilinx_spartan_3a_dsp_cost_str_map ;
			$cost_str_map{xilinx_spartan3a} = \%synplicity_xilinx_spartan3e_cost_str_map ;
			$cost_str_map{xilinx_spartan3e} = \%synplicity_xilinx_spartan3e_cost_str_map ;
			$cost_str_map{xilinx_spartan6} = \%synplicity_xilinx_spartan6_cost_str_map ;
			$cost_str_map{xilinx_spartan6l} = \%synplicity_xilinx_spartan6l_cost_str_map ;
			$cost_str_map{xilinx_virtex6} = \%synplicity_xilinx_virtex6_cost_str_map ;
			$cost_str_map{xilinx_virtex6l} = \%synplicity_xilinx_virtex6l_cost_str_map ;
			$cost_str_map{xilinx_virtex4} = \%synplicity_xilinx_spartan_3a_dsp_cost_str_map ;
			$cost_str_map{xilinx_virtex5} = \%synplicity_xilinx_spartan_3a_dsp_cost_str_map ;
		} else {
			$extract_cost_postfix='\s*:\s*(\S+)\s*out.*' ;
			$cost_str_map{xilinx_spartan_3a_dsp} = \%xst_xilinx_spartan_3a_dsp_cost_str_map ;
			$cost_str_map{xilinx_spartan3a} = \%xst_xilinx_spartan3e_cost_str_map ;
			$cost_str_map{xilinx_spartan3e} = \%xst_xilinx_spartan3e_cost_str_map ;
			$cost_str_map{xilinx_spartan6} = \%xilinx_spartan6_cost_str_map ;
			$cost_str_map{xilinx_spartan6l} = \%xilinx_spartan6l_cost_str_map ;
			$cost_str_map{xilinx_virtex6} = \%xilinx_virtex6_cost_str_map ;
			$cost_str_map{xilinx_virtex6l} = \%xilinx_virtex6l_cost_str_map ;
			$cost_str_map{xilinx_virtex4} = \%xst_xilinx_virtex4_cost_str_map ;
			$cost_str_map{xilinx_virtex5} = \%xilinx_virtex5_cost_str_map ;
		}
	} else {
		$extract_cost_postfix='\s*:\s*(\S+)\s*out.*' ;
		$cost_str_map{xilinx_spartan_3a_dsp} = \%xilinx_spartan_3a_dsp_cost_str_map ;
		$cost_str_map{xilinx_spartan3a} = \%xilinx_spartan3e_cost_str_map ;
		$cost_str_map{xilinx_spartan3e} = \%xilinx_spartan3e_cost_str_map ;
		$cost_str_map{xilinx_spartan6} = \%xilinx_spartan6_cost_str_map ;
		$cost_str_map{xilinx_spartan6l} = \%xilinx_spartan6l_cost_str_map ;
		$cost_str_map{xilinx_virtex6} = \%xilinx_virtex6_cost_str_map ;
		$cost_str_map{xilinx_virtex6l} = \%xilinx_virtex6l_cost_str_map ;
		$cost_str_map{xilinx_virtex4} = \%xilinx_virtex4_cost_str_map ;
		$cost_str_map{xilinx_virtex5} = \%xilinx_virtex5_cost_str_map ;
	}

	$cost_display_map{xilinx_spartan_3a_dsp} = \%xilinx_spartan_3a_dsp_cost_display_map ;
	$cost_display_map{xilinx_spartan3a} = \%xilinx_spartan3e_cost_display_map ;
	$cost_display_map{xilinx_spartan3e} = \%xilinx_spartan3e_cost_display_map ;
	$cost_display_map{xilinx_spartan6} = \%xilinx_spartan_3a_dsp_cost_display_map ;
	$cost_display_map{xilinx_spartan6l} = \%xilinx_spartan_3a_dsp_cost_display_map ;
	$cost_display_map{xilinx_virtex6} = \%xilinx_spartan_3a_dsp_cost_display_map ;
	$cost_display_map{xilinx_virtex6l} = \%xilinx_spartan_3a_dsp_cost_display_map ;
	$cost_display_map{xilinx_virtex4} = \%xilinx_spartan_3a_dsp_cost_display_map ;
	$cost_display_map{xilinx_virtex5} = \%xilinx_spartan_3a_dsp_cost_display_map ;
}

sub init_timing_info
{
	if($logic_synthesis_only) {
		if($SYNPLIFY_SYNTHESIS) {
			$timing_slack_search_str = 'Worst slack in design:\s*(\S+)'
		} else {
			$timing_slack_search_str = 'Slack:\s*(\S+)ns' ;
		}
	} else {
		$timing_slack_search_str = '^.*TS_clk = PERIOD TIMEGRP.*\|.*\|\s*(\S*)ns\s*\|.*\|.*\|.*' ;
		$timing_score_search_str = '^.*TS_clk = PERIOD TIMEGRP.*\|.*\|.*\|.*\|.*\|\s*(\S*)\s*' ;
	}
}

sub check_command_line {
   if (! defined $ARGV[0]) {
      print "\nusage: check_synth_results <synth results directory path>\n\n";
      die   "   ex: check_synth_results run\n";
   } else {
      print "\n";
   }
}

sub helper_get_file_info {
    my $search_dir = $_[0] ;
    my $file_ext = $_[1] ;
    my $file ;
    my $file_exists ;

    my $files = `find $search_dir -name $file_ext -print 2>/dev/null` ;
    if ( $files ){
	    $file_exists=1 ;
	    my @file_arr = split/\n/,$files ;
	    my $num=scalar(@file_arr) ;
	    $file=@file_arr[$num-1] ;
    } else {
	    $file_exists=0 ;
    }

    return ($file,$file_exists) ;
}

sub get_file_info
{
    if ($SYNPLIFY_SYNTHESIS==1) {
        ($logic_synth_file,$logic_synth_file_exists) = helper_get_file_info($run_directory,"*.srr") ;
    } else {
        ($logic_synth_file,$logic_synth_file_exists) = helper_get_file_info($run_directory,"*.syr") ;
    }
    ($ngdbuild_file,$ngdbuild_file_exists) = helper_get_file_info($run_directory,"*.bld") ; 
    ($map_file,$map_file_exists) = helper_get_file_info($run_directory,"*.mrp") ; 
    ($par_file,$par_file_exists) = helper_get_file_info($run_directory,"*.par") ; 

    my $logic_synth_only_file = "$run_directory/synthesis/.LOGIC_SYNTHESIS_ONLY" ;
    my $impl_directory = "$run_directory/implementation" ;
    if ((-e $logic_synth_only_file) and !(-e $impl_directory)) {
	    $logic_synthesis_only = 1 ;
    } else {
	    $logic_synthesis_only = 0 ;
    }
    #print "logic synthesis file = $logic_synth_file\n" ;
    #print "logic synthesis file exists = $logic_synth_file_exists\n" ;
    #print "ngdbuild file exists = $ngdbuild_file_exists\n" ;
    #print "map file exists = $map_file_exists\n" ;
    #print "par file exists = $par_file_exists\n" ;
}

sub get_area_design_info
{
	my $area_file="" ; 
	if($logic_synthesis_only) { 
		if($logic_synth_file_exists and ($logic_synth_status==PASS)) { 
			$area_file = $logic_synth_file ;
		} else {
			return 0 ;
		}
	} else {
		if($map_file_exists and ($map_status==PASS)) {
			$area_file = $map_file ;
		} else {
			return 0 ;
		}
	}

	$num_luts = get_val($cost_str_map{$technology}{LUT}.$extract_cost_postfix,$area_file,GET_AREA) ;
	$num_ffs = get_val($cost_str_map{$technology}{FF}.$extract_cost_postfix,$area_file,GET_AREA) ;
	$num_brams = get_val($cost_str_map{$technology}{BRAM}.$extract_cost_postfix,$area_file,GET_AREA) ;
	$num_dsps = get_val($cost_str_map{$technology}{DSP}.$extract_cost_postfix,$area_file,GET_AREA) ;
        return 1 ;
}

sub get_timing_info
{
	if($logic_synthesis_only) {
		if(!$logic_synth_file_exists or !($logic_synth_status==PASS)) {
			return 0 ;
		}
		$timing_slack = get_val($timing_slack_search_str,$logic_synth_file,GET_TIMING) ;
		if($timing_slack >= 0) {
			$timing_score_implication = "  ( Timing constraints met )" ;
		} else {
			$timing_score_implication = "  ( Timing constraints not satisfied )" ;
		}
	} else {
		if(!$par_file_exists or !($par_status==PASS)) {
			return 0 ;
		}
		$timing_slack = get_val($timing_slack_search_str,$par_file,GET_TIMING) ;
		$timing_score = get_val($timing_score_search_str,$par_file,GET_TIMING) ;
		if($timing_score == 0) {
			$timing_score_implication = "  ( Timing constraints met )" ;
		} else {
			$timing_score_implication = "  ( Timing constraints not satisfied )" ;
		}
	}

	return 1 ;
}

sub return_exit_status
{
    if ($timing_slack < 0.0) {
        $timing_status = FAIL ;
    } else {
            $timing_status = PASS ;
    }

    if($logic_synthesis_only) {
	    if ($logic_synth_status == PASS) {
		    $run_flow_status = PASS ;
	    } else {
		    $run_flow_status = FAIL ;
	    }
    } else {
	    if ($logic_synth_status == PASS and $ngdbuild_status== PASS and 
			    $map_status == PASS and $par_status == PASS) {
		    $run_flow_status = PASS ;
	    } else {
		    $run_flow_status = FAIL ;
	    }
    }

    if ($timing_status==PASS and  $run_flow_status==PASS) {
        exit 0 ;
    } else {
        exit 1 ;
    }
}

sub get_message_length
{
    my $message = $_[0] ;

    my $length = length $message;
    $length = $spacing - $length;
    $length = " " x $length;

    return $length ;
}

sub print_status_message
{
    my $status  = $_[0] ;
    my $message = $_[1] ;

    my $length  = get_message_length($message) ;
    if ( $status==PASS ){ 
        print "$length $message : Completed\n";
    } elsif ( $status==FAIL ) {
        print "$length $message : Failed\n";
    } elsif ( $status==NOT_RUN ) {
        print "$length $message : Not run\n";
    }
}

sub print_status
{
    print_status_message($logic_synth_status,$logic_synth_message) ;
    if($logic_synthesis_only) {
        print "\n";
        return ;
    } else {
        print_status_message($ngdbuild_status,$ngdbuild_message) ;
        print_status_message($map_status,$map_message) ;
        print_status_message($par_status,$par_message) ;
        print "\n";
    }
}

##### Remove whitespaces and commas
sub remove_useless_characters
{
    my $input_string = $_[0] ;

    $input_string =~ s/\s*(\S+)\s*/$1/ ;
    $input_string =~ s/,//g ;

    return $input_string ;
}

sub print_area_report
{
    $num_luts = remove_useless_characters($num_luts) ;
    $num_ffs = remove_useless_characters($num_ffs) ;
    $num_dsps = remove_useless_characters($num_dsps) ;
    $num_brams = remove_useless_characters($num_brams) ;

    my $length = get_message_length($area_message) ;
    print "$length $area_message : ( $cost_display_map{$technology}{LUT} = $num_luts, $cost_display_map{$technology}{FF} = $num_ffs, $cost_display_map{$technology}{DSP} = $num_dsps, $cost_display_map{$technology}{BRAM} = $num_brams )\n";
    
    print "\n";
}

sub print_timing_report
{
    my $length1 = get_message_length($slack_message) ;
    my $length2 = get_message_length($timing_score_message) ;

    $timing_slack = remove_useless_characters($timing_slack) ;
    $timing_score = remove_useless_characters($timing_score) ;

    if($timing_slack ne 'NA') {
        if($logic_synthesis_only) {
            print "$length1 $slack_message : $timing_slack ns";
            print "$timing_score_implication\n";
        } else {
            print "$length1 $slack_message : $timing_slack ns\n";
            print "$length2 $timing_score_message : $timing_score $timing_score_implication\n";
        }
    } else {
        my $note = "NOTE" ;
        my $slack_na_message = "Slack not available." ;
        my $note_length = get_message_length($note) ;
        print "$note_length $note : $slack_na_message\n" ;
    }
    print "\n";
}

sub get_val
{
    my $search_string = $_[0] ;
    my $file = $_[1] ; #has to be switched incase of logic synthesis
    my $type = $_[2] ;

    #print "search_string = $search_string\n" ;
    #print "file = $file\n" ;
    
    open (FILE, $file) || die "\nERROR: can't open $file file for reading\n";
    my $line ;
    my $other_val = 0 ;
    my $timing_val = "NA" ;
    while (chomp ($line = <FILE>)) {
        if ($line =~ s/$search_string/$1/) {
            #print "get_val = $line\n" ;
            if($type == GET_AREA || $type == GET_ERROR) {
                $other_val += remove_useless_characters($line) ;
            } elsif($type == GET_TIMING) {
                $timing_val = $line ;
            } else {
                print "InternalError: Unsupported type used as parameter in get_val function\n" ;
                exit 1 ;
            }
        }
    }
    
    if($type == GET_AREA || $type == GET_ERROR) {
        return $other_val ;
    } elsif($type == GET_TIMING) {
        return $timing_val ;
    } else {
        print "InternalError: Unsupported type used as parameter in get_val function\n" ;
        exit 1 ;
    }
}

sub check_status
{
    my $error_string_postfix = '\s*:\s*(\S+).*' ;
    if( $logic_synth_file_exists==1 ) {
        my $num_errors ;
        if ($SYNPLIFY_SYNTHESIS==1) {
            chomp (my @errors = `grep "\@E:" $logic_synth_file`);
            $num_errors = @errors=="" ? 0 : 1 ;
        } else {
            $num_errors = get_val("Number of errors".$error_string_postfix,$logic_synth_file,GET_ERROR) ;
        }

        if ( $num_errors==0 ) {
            $logic_synth_status=PASS ;
        } else {
            $logic_synth_status=FAIL ;
        }
    }

    if($logic_synthesis_only) {
	    $ngdbuild_status=NOT_RUN ;
	    $map_status=NOT_RUN ;
	    $par_status=NOT_RUN ;
    } else {
	    if ( $ngdbuild_file_exists ) {
		    my $num_errors = get_val("Number of errors".$error_string_postfix,$ngdbuild_file,GET_ERROR) ;
		    if ( $num_errors==0 ) {
			    $ngdbuild_status=PASS ;
		    } else {
			    $ngdbuild_status=FAIL ;
		    }
	    } else {
		    if ($logic_synth_status==PASS) {
			    $ngdbuild_status=FAIL ;
		    } else {
			    $ngdbuild_status=NOT_RUN ;
		    }
	    }

	    if ( $map_file_exists ) {
		    my $num_errors = get_val("Number of errors".$error_string_postfix,$map_file,GET_ERROR) ;
		    if ( $num_errors==0 ) {
			    $map_status=PASS ;
		    } else {
			    $map_status=FAIL ;
		    }
	    } else {
		    if ( $ngdbuild_status==PASS) {
			    $map_status=FAIL ;
		    } else {
			    $map_status=NOT_RUN ;
		    }
	    }

	    if ( $par_file_exists ) {
		    my $num_errors = get_val("Number of error messages".$error_string_postfix,$par_file,GET_ERROR) ;
		    if ( $num_errors==0 ) {
			    $par_status=PASS ;
		    } else {
			    $par_status=FAIL ;
		    }
	    } else {
		    if ( $map_status==PASS) {
			    $par_status=FAIL ;
		    } else {
			    $par_status=NOT_RUN ;
		    }
	    }
    }
}

#print "check command line\n" ;
check_command_line ;
#print "get file info\n" ;
get_file_info ;
init_cost_info ;
init_timing_info ;
#print "check status\n" ;
check_status ;
#print "get area design info\n" ;
my $area_info_exists = get_area_design_info ;
#print "get timing info\n" ;
my $timing_info_exists = get_timing_info ; 
#print "print status\n" ;
print_status ;
#print "print area report\n" ;
if($area_info_exists) {
	print_area_report ; 
}
#print "print timing report\n" ;
if($timing_info_exists) {
	print_timing_report ;
}
#print "exit status" ;
&return_exit_status ;
