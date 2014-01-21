#include <cu/cu.h>
#include <boruvka/hdf5.h>
#include <boruvka/dbg.h>
#include <boruvka/alloc.h>

static bor_h5file_t hf;
static bor_h5dset_t *dset;

TEST(hdf5SetUp)
{
    // open data file
    assertEquals(borH5FileOpen(&hf, "hdf5.h5", "r"), 0);

    // open dataset
    dset = borH5DatasetOpen(&hf, "train/x");
    assertNotEquals(dset, NULL);

    assertEquals(dset->ndims, 2);
    assertEquals(dset->dims[0], 100);
    assertEquals(dset->dims[1], 40000);
}

TEST(hdf5TearDown)
{
    assertEquals(borH5FileClose(&hf), 0);
}

TEST(hdf5OpenR)
{
    size_t size;
    float *dataf;
    int *datai;

    // get number of elements in dataset
    size = dset->num_elements;
    assertEquals(dset->num_elements, 4000000);

    assertEquals(borH5DatasetLoadFloat(dset, &dataf), size);
    assertTrue(borEq(dataf[0], 0.0078431377f));
    assertTrue(borEq(dataf[size - 1], 0.0117647061f));
    assertTrue(borEq(dataf[10], 0.0392156877));
    assertTrue(borEq(dataf[41000], 0.0156862754));

    assertEquals(borH5DatasetLoadInt(dset, &datai), size);
    assertEquals(datai[0], 0);
    assertEquals(datai[size - 1], 0);
}

TEST(hdf5Vec)
{
    bor_gsl_vector *vec;

    vec = borH5DatasetLoadVec(dset);
    assertEquals(vec->size, 4000000);
    assertTrue(borEq(bor_gsl_vector_get(vec, 0), 0.0078431377f));
    assertTrue(borEq(bor_gsl_vector_get(vec, 3999999), 0.0117647061f));
    assertTrue(borEq(bor_gsl_vector_get(vec, 10), 0.0392156877));
    assertTrue(borEq(bor_gsl_vector_get(vec, 41000), 0.0156862754));
}

TEST(hdf5Mat)
{
    bor_h5dset_t *dset;
    bor_gsl_matrix *mat;

    // try open identical dataset once more
    dset = borH5DatasetOpen(&hf, "train/x");
    assertNotEquals(dset, NULL);

    assertEquals(dset->ndims, 2);
    assertEquals(dset->dims[0], 100);
    assertEquals(dset->dims[1], 40000);

    mat = borH5DatasetLoadMat(dset);
    assertEquals(mat->size1, 100);
    assertEquals(mat->size2, 40000);
    assertTrue(borEq(bor_gsl_matrix_get(mat, 0, 0), 0.0078431377f));
    assertTrue(borEq(bor_gsl_matrix_get(mat, 99, 39999), 0.0117647061f));
    assertTrue(borEq(bor_gsl_matrix_get(mat, 0, 10), 0.0392156877));
    assertTrue(borEq(bor_gsl_matrix_get(mat, 1, 1000), 0.0156862754));

    assertEquals(borH5DatasetClose(dset), 0);
}

TEST(hdf5MatRowRange)
{
    bor_gsl_matrix *mat;

    mat = borH5DatasetLoadMatRowRange(dset, 1, 3);
    assertEquals(mat->size1, 3);
    assertEquals(mat->size2, 40000);
    assertTrue(borEq(bor_gsl_matrix_get(mat, 0, 1000), 0.0156862754));

    mat = borH5DatasetLoadMatRowRange(dset, 0, 2);
    assertEquals(mat->size1, 2);
    assertEquals(mat->size2, 40000);
    assertTrue(borEq(bor_gsl_matrix_get(mat, 0, 0), 0.0078431377f));
    assertTrue(borEq(bor_gsl_matrix_get(mat, 0, 10), 0.0392156877));
    assertTrue(borEq(bor_gsl_matrix_get(mat, 1, 1000), 0.0156862754));


    mat = borH5DatasetLoadMatRowRange(dset, 95, 5);
    assertEquals(mat->size1, 5);
    assertEquals(mat->size2, 40000);
    assertTrue(borEq(bor_gsl_matrix_get(mat, 4, 39999), 0.0117647061f));
}

TEST(hdf5Region)
{
    bor_real_t *data;
    size_t start[2], count[2], size;

    start[0] = 0;
    start[1] = 10;
    count[0] = 2;
    count[1] = 1000;
    size = borH5DatasetLoadRegionReal(dset, start, count, &data);
    assertEquals(size, 2000);
    assertTrue(borEq(data[0], 0.0392156877));
    assertTrue(borEq(data[1990], 0.0156862754));
}
