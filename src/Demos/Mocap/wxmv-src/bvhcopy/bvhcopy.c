/*
 *  bvhcopy.c
 *
 *  Purpose: Extracts a part of a BVH motion file and copies it to a new one.
 *           Controlled by commandline arguments explained in show_usage()
 *           function.
 *
 *  Created:   Jaroslav Semancik, 16/12/2003
 */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#define ROUND(x)  floor((x) + 0.5)

enum { FALSE, TRUE };
enum { FRAME, FRACTION, TIME };

int start_frame;            /* copy from start_frame (frames are numbered from 0) */
int end_frame;              /* copy to end_frame */
int n_frames;               /* # of frames in input file */
int copied_frames;          /* actually copied frames */

float frac_start_frame;     /* fractional start frame - in [0.0, 1.0] */
float frac_end_frame;       /* fractional end frame - in [0.0, 1.0] */
float start_time;           /* start time */
float end_time;             /* end time */
float delta;                /* time difference between two consecutive frames */

int start_type, end_type;   /* type of given start/end format */

char in_filename[1001];     /* input filename */
char out_filename[1001];    /* output filename */
FILE *in_file, *out_file;   /* input and output file */


int parse_options(int n, char **arg);
void show_usage();


int main(int argc, char **argv)
{
    char line[10001];
    int i;

    /* set defaults - copy the entire motion to out.bvh */
    start_type = FRACTION;
    frac_start_frame = 0.0;
    end_type = FRACTION;
    frac_end_frame = 1.0;
    strcpy(in_filename, "");
    strcpy(out_filename, "out.bvh");

    /* without arguments show usage */
    if (argc <= 1)
    {
        show_usage();
        return 0;
    }

    /* parse options */
    if (!parse_options(argc, argv))
        return -1;

    /* open files */
    in_file = fopen(in_filename, "rt");
    if (!in_file)
    {
    	printf("Error: Cannot open input file %s", in_filename);
    	return -1;
    }

    out_file = fopen(out_filename, "wt");
    if (!out_file)
    {
    	printf("Error: Cannot create output file %s", out_filename);
    	fclose(in_file);
    	return -1;
    }

    /* copy HIERARCHY section */
    do
    {
    	if (feof(in_file)) break;
        fgets(line, 10000, in_file);
        fputs(line, out_file);
    }
    while (!(strstr(line, "MOTION") == line));

    if (feof(in_file))
    {
    	printf("Error: Incomplete input file, the MOTION section missing");
    	fclose(in_file);
    	fclose(out_file);
    	return -1;
    }

    /* read number of frames and frame time */
    fscanf(in_file, "%*s %d\n", &n_frames);
    fscanf(in_file, "%*s %*s %f\n", &delta);

    /* set start and end frames to cut */
    if (start_type == FRACTION)
        start_frame = ROUND(frac_start_frame * (n_frames - 1));
    else if (start_type == TIME)
        start_frame = ROUND(start_time / delta);

    if (end_type == FRACTION)
        end_frame = ROUND(frac_end_frame * (n_frames - 1));
    else if (end_type == TIME)
        end_frame = ROUND(end_time / delta);

    if (start_frame < 0)
        start_frame = 0;

    if (end_frame > n_frames - 1)
        end_frame = n_frames - 1;

    if (end_frame < start_frame)  end_frame = start_frame - 1;

    /* write number of frames and frame time */
    fprintf(out_file, "Frames: %d\n", end_frame - start_frame + 1);
    fprintf(out_file, "Frame Time: %f\n", delta);

    /* copy frames */
    for (i = 0; i < start_frame; i++)
    {
        if (feof(in_file)) break;
        fgets(line, 10000, in_file);
    }

    copied_frames = 0;
    for (i = start_frame; i <= end_frame; i++)
    {
    	if (feof(in_file)) break;
        fgets(line, 10000, in_file);
        fputs(line, out_file);
        copied_frames++;
    }

    /* close files */
    fclose(in_file);
    fclose(out_file);

    /* report status */
    if (copied_frames == end_frame - start_frame + 1)
        printf("OK");
    else
        printf("Warning: Incorrect value 'Frames' in input file, check number of copied frames.");

    printf("\nMotion with %d frames (%d..%d) from %s copied to %s\n",
        copied_frames, start_frame, start_frame + copied_frames - 1,
        in_filename, out_filename);
    return 0;
}


void show_usage()
{
    printf("\n*** BVH cutter - extracts a part of .bvh motion file. ***\n");
    printf("    by Jaroslav Semancik, 2003\n\n");
    printf("Usage:\n");
    printf("  bvhcopy.exe [OPTIONS] <input_file> [OPTIONS]\n");
    printf("  Options:\n");
    printf("    -s #[%%/s]      start frame number #  (default: 0 - the first frame)\n");
    printf("    -e #[%%/s]      end frame number #    (default: the last frame)\n");
    printf("    -o <filename>  output file name      (default: out.bvh)\n");
    printf("    -h             help, show usage\n");
    printf("\n");
    printf("    where format of # is: an integer frame number, or\n");
    printf("                          percentage (float) followed by %% sign, or\n");
    printf("                          time in seconds (float) followed by letter s\n");
    printf("\n");
    printf("Examples:\n");
    printf("  bvhcopy.exe walk.bvh -s 20 -e 100 -o step.bvh   - extract frames 20 to 100\n");
    printf("  bvhcopy.exe jump.bvh -e 33.33%% -o jump1.bvh     - extract first third\n");
    printf("  bvhcopy.exe run.bvh -s 1.5s -e 50%% -o leap.bvh  - from time 1.5s to the half\n");
    printf("\n");
}


int parse_options(int n, char **arg)
{
    int i;

    i = 1;
    while (i < n)
    {
    	/* start frame option -s */
    	if (strcmp(arg[i], "-s") == 0)
    	{
    		i++;
            if (i < n)
            {
                /* start given by percentage */
                if (strchr(arg[i], '%'))
                {
                	start_type = FRACTION;
                	frac_start_frame = atof(arg[i]) / 100.0;
                }
                /* start given by time */
                else if (strchr(arg[i], 's'))
                {
                	start_type = TIME;
                	start_time = atof(arg[i]);
                }
                /* start given by frame number */
                else
                {
                	start_type = FRAME;
                    start_frame = atoi(arg[i]);
                }
            }
        }

    	/* end frame option -e */
    	else if (strcmp(arg[i], "-e") == 0)
        {
        	i++;
        	if (i < n)
        	{
                /* end given by percentage */
        	    if (strchr(arg[i], '%'))
                {
                	end_type = FRACTION;
                	frac_end_frame = atof(arg[i]) / 100.0;
                }
                /* end given by time */
                else if (strchr(arg[i], 's'))
                {
                	end_type = TIME;
                	end_time = atof(arg[i]);
                }
                /* end given by frame number */
                else
                {
                	end_type = FRAME;
                    end_frame = atoi(arg[i]);
                }
            }
        }

       	/* output filename option -o */
    	else if (strcmp(arg[i], "-o") == 0)
        {
        	i++;
        	if (i < n)
                strcpy(out_filename, arg[i]);
        }

       	/* help option -h */
    	else if (strcmp(arg[i], "-h") == 0)
            show_usage();

        /* invalid option */
        else if (arg[i][0] == '-')
        {
        	printf("Error: Invalid option %s\n", arg[i]);
        	return FALSE;
        }

        /* argument is not an option, so it is an input filename */
        else
            strcpy(in_filename, arg[i]);

        // move to next argument */
        i++;
    }

    if (in_filename == "")
    {
    	printf("Error: No input file given\n");
    	return FALSE;
    }

    /* options are OK */
    return TRUE;
}

