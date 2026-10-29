#!/usr/bin/env perl
use strict;
use warnings;
use File::Basename qw(dirname);
use File::Spec;

# This localization branch has a known upstream baseline. Tags present in a
# server checkout must not accidentally identify it as an older Crawl release.
sub crawl_source_version {
    my ($revision, $base_only) = @_;
    my $dir = dirname(__FILE__);
    open my $base_file, '<', "$dir/version_base" or die $!;
    my $base = <$base_file>;
    close $base_file;
    $base =~ /\A(\d+\.\d+\.\d+) ([a-f0-9]{40})\s*\z/
        or die "Malformed version_base\n";
    my ($version, $commit) = ($1, $2);
    return $version if $base_only;
    $revision ||= 'HEAD';
    $revision =~ /\A[A-Za-z0-9_.\/-]+\z/ or die "Invalid version revision\n";

    my $git = sub {
        # Missing Git metadata is expected for source archives.
        open my $saved_stderr, '>&', \*STDERR or die $!;
        open STDERR, '>', File::Spec->devnull() or die $!;
        my $opened = open my $pipe, '-|', 'git', @_;
        open STDERR, '>&', $saved_stderr or die $!;
        return unless $opened;
        local $/;
        my $text = <$pipe>;
        close $pipe or return;
        return unless defined $text;
        $text =~ s/\s+\z//;
        return $text;
    };
    my $head = $git->('rev-parse', '--short=10', $revision);
    if (defined $head && $head =~ /\A[a-f0-9]{10,}\z/) {
        my $ancestor = $git->('merge-base', $commit, $revision);
        if (defined $ancestor && $ancestor eq $commit) {
            my $count = $git->('rev-list', '--count', "$commit..$revision");
            defined $count && $count =~ /\A\d+\z/ or die "Cannot count source revisions\n";
            return $count == 0 ? $version : "$version-$count-g$head";
        }
        # A shallow clone cannot determine distance from the baseline. Preserve
        # the correct release and exact revision without inventing a count.
        return "$version-shallow-g$head"
            if ($git->('rev-parse', '--is-shallow-repository') || '') eq 'true';
        die "Source revision is not descended from version_base\n";
    }
    if (open my $release, '<', "$dir/release_ver") {
        my $text = <$release>;
        close $release;
        chomp $text;
        return $text if $text =~ /\A\Q$version\E(?:-|\z)/;
    }
    return $version;
}

unless (caller) {
    my $base_only = @ARGV && $ARGV[0] eq '--base-version';
    shift @ARGV if $base_only;
    print crawl_source_version($ARGV[0], $base_only), "\n";
}
1;
