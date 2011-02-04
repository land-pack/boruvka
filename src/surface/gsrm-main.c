/***
 * fermat
 * -------
 * Copyright (c)2011 Daniel Fiser <danfis@danfis.cz>
 *
 *  This file is part of fermat.
 *
 *  Distributed under the OSI-approved BSD License (the "License");
 *  see accompanying file BDS-LICENSE for details or see
 *  <http://www.opensource.org/licenses/bsd-license.php>.
 *
 *  This software is distributed WITHOUT ANY WARRANTY; without even the
 *  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *  See the License for more information.
 */

#include <getopt.h>
#include <fermat/surface/gsrm.h>
#include <fermat/parse.h>


typedef enum {
    HELP = 'h',
    OUTFILE = 'o',
    VERBOSE = 'v',

    EPSILON_N = 1000,
    EPSILON_B,
    LAMBDA,
    BETA,
    ALPHA,
    AGE_MAX,
    MAX_NODES,
    MIN_DANGLE,
    MAX_ANGLE,
    ANGLE_MERGE_EDGES,

    DUMP_TRIANGLES,

    NUM_CUBES
} options_enum;

static struct option options[] = {
    { "help", no_argument, NULL, HELP },

    { "epsilon-n", required_argument, NULL, EPSILON_N },
    { "epsilon-b", required_argument, NULL, EPSILON_B },
    { "lambda",    required_argument, NULL, LAMBDA },
    { "beta",      required_argument, NULL, BETA },
    { "alpha",     required_argument, NULL, ALPHA },
    { "age-max",   required_argument, NULL, AGE_MAX },
    { "max-nodes", required_argument, NULL, MAX_NODES },
    { "num-cubes", required_argument, NULL, NUM_CUBES },
    { "min-dangle", required_argument, NULL, MIN_DANGLE },
    { "max-angle", required_argument, NULL, MAX_ANGLE },
    { "angle-merge-edges", required_argument, NULL, ANGLE_MERGE_EDGES },

    { "dump-triangles", required_argument, NULL, DUMP_TRIANGLES },

    { "outfile", required_argument, NULL, OUTFILE },

    { NULL, 0, NULL, 0}
};

#define DUMP_TRIANGLES_FN_LEN 100
static fer_gsrm_t *gsrm;
static const char *is_fn = NULL;
static const char *outfile_fn;
static FILE *dump_triangles = NULL;
static char dump_triangles_fn[DUMP_TRIANGLES_FN_LEN + 1] = "";

static void usage(int argc, char *argv[], const char *opt_msg);
static void readOptions(int argc, char *argv[]);
static void printAttrs(void);

int main(int argc, char *argv[])
{
    fer_mesh3_t *mesh;
    size_t islen;
    FILE *outfile;
    fer_timer_t timer;

    gsrm = ferGSRMNew();
    ferGSRMSetVerbosity(gsrm, 1);

    readOptions(argc, argv);
    printAttrs();

    // open output file
    if (outfile_fn == NULL){
        outfile = stdout;
    }else{
        outfile = fopen(outfile_fn, "w");
        if (outfile == NULL){
            fprintf(stderr, "Can't open '%s' for writing!\n", outfile_fn);
            return -1;
        }
    }

    ferTimerStart(&timer);
    ferTimerStopAndPrintElapsed(&timer, stderr, " Reading input signals:\n");
    ferTimerStopAndPrintElapsed(&timer, stderr, "   -- '%s'...\n", is_fn);
    islen = ferGSRMAddInputSignals(gsrm, is_fn);
    ferTimerStopAndPrintElapsed(&timer, stderr, "     --  Added %d input signals.\n", islen);
    fprintf(stderr, "\n");

    if (ferGSRMRun(gsrm) == 0){
        ferGSRMPostprocess(gsrm);

        ferTimerStart(&timer);

        mesh = ferGSRMMesh(gsrm);
        ferMesh3DumpSVT(mesh, outfile, "Result");

        if (ferGSRMVerbosity(gsrm) >= 2){
            fprintf(stderr, "\n");
            ferTimerStopAndPrintElapsed(&timer, stderr, " Mesh dumped to '%s'.\n",
                                        (outfile == stdout ? "stdout" : outfile_fn));
        }

        if (dump_triangles != NULL){
            ferMesh3DumpTriangles(mesh, dump_triangles);
            fclose(dump_triangles);

            if (ferGSRMVerbosity(gsrm) >= 2){
                ferTimerStopAndPrintElapsed(&timer, stderr,
                                            " Mesh dumped as triangles into '%s'.\n",
                                            dump_triangles_fn);
            }
        }
    }

    ferGSRMDel(gsrm);


    // close output file
    if (outfile != stdout)
        fclose(outfile);

    return 0;
}


void readOptions(int argc, char *argv[])
{
    int c, option_index;
    long iv;
    fer_real_t fv;
    int num_cubes_set = 0;
    fer_gsrm_param_t *param;

    param = ferGSRMParam(gsrm);

    while ((c = getopt_long(argc, argv, "hvo:", options, &option_index)) != -1){
        switch(c){
            case HELP:
                usage(argc, argv, NULL);
                break;
            case VERBOSE:
                ferGSRMSetVerbosity(gsrm, ferGSRMVerbosity(gsrm) + 1);
                break;
            case EPSILON_N:
                if (ferParseReal(optarg, optarg + strlen(optarg), &fv, NULL) != 0)
                    usage(argc, argv, "epsilon-n must be floating point "
                                      "number");
                param->en = fv;
                break;
            case EPSILON_B:
                if (ferParseReal(optarg, optarg + strlen(optarg), &fv, NULL) != 0)
                    usage(argc, argv, "epsilon-b must be floating point "
                                      "number");
                param->eb = fv;
                break;
            case LAMBDA:
                if (ferParseLong(optarg, optarg + strlen(optarg), &iv, NULL) != 0)
                    usage(argc, argv, "lambda must be fixed point number");
                param->lambda = iv;
                break;
            case BETA:
                if (ferParseReal(optarg, optarg + strlen(optarg), &fv, NULL) != 0)
                    usage(argc, argv, "beta must be floating point number");
                param->beta = fv;
                break;
            case ALPHA:
                if (ferParseReal(optarg, optarg + strlen(optarg), &fv, NULL) != 0)
                    usage(argc, argv, "alpha must be floating point number");
                param->alpha = fv;
                break;
            case AGE_MAX:
                if (ferParseLong(optarg, optarg + strlen(optarg), &iv, NULL) != 0)
                    usage(argc, argv, "age-max must be fixed point number");
                param->age_max = iv;
                break;
            case MAX_NODES:
                if (ferParseLong(optarg, optarg + strlen(optarg), &iv, NULL) != 0)
                    usage(argc, argv, "max-nodes must be fixed point number");
                param->max_nodes = iv;
                break;
            case NUM_CUBES:
                if (ferParseLong(optarg, optarg + strlen(optarg), &iv, NULL) != 0)
                    usage(argc, argv, "num-cubes must be fixed point number");
                param->num_cubes = iv;
                num_cubes_set = 1;
                break;

            case MIN_DANGLE:
                if (ferParseReal(optarg, optarg + strlen(optarg), &fv, NULL) != 0)
                    usage(argc, argv, "min-dangle must be float");
                param->min_dangle = fv;
                break;
            case MAX_ANGLE:
                if (ferParseReal(optarg, optarg + strlen(optarg), &fv, NULL) != 0)
                    usage(argc, argv, "max-angle must be float");
                param->max_angle = fv;
                break;
            case ANGLE_MERGE_EDGES:
                if (ferParseReal(optarg, optarg + strlen(optarg), &fv, NULL) != 0)
                    usage(argc, argv, "angle-merge-edges must be float");
                param->angle_merge_edges = fv;
                break;

            case DUMP_TRIANGLES:
                    dump_triangles = fopen(optarg, "w");
                    if (dump_triangles == NULL)
                        usage(argc, argv, "can't open file for dump-triangles");
                    strncpy(dump_triangles_fn, optarg, DUMP_TRIANGLES_FN_LEN);
                    break;

            case OUTFILE:
                if (strcmp(optarg, "stdout") == 0){
                    outfile_fn = NULL;
                }else{
                    outfile_fn = optarg;
                }
                break;

            default:
                usage(argc, argv, "");
        }
    }
    if (argc - optind != 1){
        usage(argc, argv, "filename must be specified");
    }
    is_fn = argv[argc - 1];

    if (!num_cubes_set){
        param->num_cubes = param->max_nodes;
    }
}

static void usage(int argc, char *argv[], const char *opt_msg)
{
    if (opt_msg != NULL){
        fprintf(stderr, "%s\n", opt_msg);
    }

    fprintf(stderr, "\n");
    fprintf(stderr, "Usage %s [ options ] filename\n", argv[0]);
    fprintf(stderr, "   Options: --epsilon-b float  Winner learning rate\n");
    fprintf(stderr, "            --epsilon-n float  Winner's neighbors learning rate\n");
    fprintf(stderr, "            --lambda    int    Steps in cycle\n");
    fprintf(stderr, "            --beta      float  Error counter decreasing rate\n");
    fprintf(stderr, "            --alpha     float  Error counter decreasing rate\n");
    fprintf(stderr, "            --age-max   int\n");
    fprintf(stderr, "            --max-nodes int    Stop Criterium\n");
    fprintf(stderr, "            --num-cubes int    Number of cubes (default is value of --max-nodes)\n");
    fprintf(stderr, "\n");
    fprintf(stderr, "            --min-dangle        float  Minimal dihedral angle between faces\n");
    fprintf(stderr, "            --max-angle         float  Maximal angle in cusp of face\n");
    fprintf(stderr, "            --angle-merge-edges float  Minimal angle between edges to merge them\n");
    fprintf(stderr, "\n");

    fprintf(stderr, "\n");

    fprintf(stderr, "            --outfile / -o   filename Filename where will be dumped resulting mesh (stdout is default)\n");
    fprintf(stderr, "            --dump-triangles filename Filename where will be stored triangles from reconstructed object.\n");
    fprintf(stderr, "\n");
    fprintf(stderr, "            -v / -vv / ...  Increases verbosity\n");
    fprintf(stderr, "\n");

    exit(-1);
}

void printAttrs(void)
{
    fer_gsrm_param_t *param;

    param = ferGSRMParam(gsrm);

    fprintf(stderr, "Attributes:\n");
    fprintf(stderr, "    lambda:    %d\n", param->lambda);
    fprintf(stderr, "    eb:        %f\n", (float)param->eb);
    fprintf(stderr, "    en:        %f\n", (float)param->en);
    fprintf(stderr, "    alpha      %f\n", (float)param->alpha);
    fprintf(stderr, "    beta:      %f\n", (float)param->beta);
    fprintf(stderr, "    age_max:   %d\n", param->age_max);
    fprintf(stderr, "    max nodes: %d\n", param->max_nodes);
    fprintf(stderr, "    num cubes: %d\n", param->num_cubes);
    fprintf(stderr, "\n");
    fprintf(stderr, "    min d. angle:  %f\n", param->min_dangle);
    fprintf(stderr, "    max angle:     %f\n", param->max_angle);
    fprintf(stderr, "    ang. merge e.: %f\n", param->angle_merge_edges);
    fprintf(stderr, "\n");
    fprintf(stderr, "    input signals: %s\n", is_fn);
    fprintf(stderr, "\n");
    fprintf(stderr, "    outfile: %s\n", (outfile_fn == NULL ? "stdout" : outfile_fn));
    fprintf(stderr, "\n");
}
