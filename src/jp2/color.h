#pragma once
#include <openjpeg.h>

static void
sycc_to_rgb (int  offset,
             int  upb,
             int  y,
             int  cb,
             int  cr,
             int *out_r,
             int *out_g,
             int *out_b)
{
  int r, g, b;

  cb -= offset;
  cr -= offset;
  r = y + (int) (1.402 * (float) cr);

  if (r < 0)
    r = 0;
  else if (r > upb)
    r = upb;
  *out_r = r;

  g = y - (int) (0.344 * (float) cb + 0.714 * (float) cr);
  if (g < 0)
    g = 0;
  else if (g > upb)
    g = upb;
  *out_g = g;

  b = y + (int) (1.772 * (float) cb);
  if (b < 0)
    b = 0;
  else if (b > upb)
    b = upb;
  *out_b = b;
}

static bool
sycc420_to_rgb (opj_image_t *img)
{
  int *d0, *d1, *d2, *r, *g, *b, *nr, *ng, *nb;
  const int *y, *cb, *cr, *ny;
  size_t maxw, maxh, max, offx, loopmaxw, offy, loopmaxh;
  int offset, upb;
  size_t i;

  upb = (int) img->comps[0].prec;
  offset = 1 << (upb - 1);
  upb = (1 << upb) - 1;

  maxw = (size_t) img->comps[0].w;
  maxh = (size_t) img->comps[0].h;
  max = maxw * maxh;

  y = img->comps[0].data;
  cb = img->comps[1].data;
  cr = img->comps[2].data;

  d0 = r = (int *) malloc (sizeof (int) * max);
  d1 = g = (int *) malloc (sizeof (int) * max);
  d2 = b = (int *) malloc (sizeof (int) * max);

  if (r == NULL || g == NULL || b == NULL)
    {
      printf("malloc() failed in sycc420_to_rgb()\n");
      goto out;
    }

  /* if img->x0 is odd, then first column shall use Cb/Cr = 0 */
  offx = img->x0 & 1U;
  loopmaxw = maxw - offx;
  /* if img->y0 is odd, then first line shall use Cb/Cr = 0 */
  offy = img->y0 & 1U;
  loopmaxh = maxh - offy;

  if (offy > 0U)
    {
      size_t j;

      for (j = 0; j < maxw; ++j)
        {
          sycc_to_rgb (offset, upb, *y, 0, 0, r, g, b);
          ++y;
          ++r;
          ++g;
          ++b;
        }
    }

  for (i = 0U; i < (loopmaxh & ~(size_t) 1U); i += 2U)
    {
      size_t j;

      ny = y + maxw;
      nr = r + maxw;
      ng = g + maxw;
      nb = b + maxw;

      if (offx > 0U)
        {
          sycc_to_rgb (offset, upb, *y, 0, 0, r, g, b);
          ++y;
          ++r;
          ++g;
          ++b;

          sycc_to_rgb (offset, upb, *ny, *cb, *cr, nr, ng, nb);
          ++ny;
          ++nr;
          ++ng;
          ++nb;
        }

      for (j = 0; j < (loopmaxw & ~(size_t) 1U); j += 2U)
        {
          sycc_to_rgb (offset, upb, *y, *cb, *cr, r, g, b);
          ++y;
          ++r;
          ++g;
          ++b;

          sycc_to_rgb (offset, upb, *y, *cb, *cr, r, g, b);
          ++y;
          ++r;
          ++g;
          ++b;

          sycc_to_rgb (offset, upb, *ny, *cb, *cr, nr, ng, nb);
          ++ny;
          ++nr;
          ++ng;
          ++nb;

          sycc_to_rgb (offset, upb, *ny, *cb, *cr, nr, ng, nb);
          ++ny;
          ++nr;
          ++ng;
          ++nb;

          ++cb;
          ++cr;
        }

      if (j < loopmaxw)
        {
          sycc_to_rgb (offset, upb, *y, *cb, *cr, r, g, b);
          ++y;
          ++r;
          ++g;
          ++b;

          sycc_to_rgb (offset, upb, *ny, *cb, *cr, nr, ng, nb);
          ++ny;
          ++nr;
          ++ng;
          ++nb;

          ++cb;
          ++cr;
        }

      y += maxw;
      r += maxw;
      g += maxw;
      b += maxw;
    }

  if (i < loopmaxh)
    {
      size_t j;

      for (j = 0U; j < (maxw & ~(size_t) 1U); j += 2U)
        {
          sycc_to_rgb (offset, upb, *y, *cb, *cr, r, g, b);
          ++y;
          ++r;
          ++g;
          ++b;

          sycc_to_rgb (offset, upb, *y, *cb, *cr, r, g, b);
          ++y;
          ++r;
          ++g;
          ++b;

          ++cb;
          ++cr;
        }

      if (j < maxw)
        {
          sycc_to_rgb (offset, upb, *y, *cb, *cr, r, g, b);
        }
    }

  free (img->comps[0].data);
  img->comps[0].data = d0;
  free (img->comps[1].data);
  img->comps[1].data = d1;
  free (img->comps[2].data);
  img->comps[2].data = d2;

  img->comps[1].w = img->comps[2].w = img->comps[0].w;
  img->comps[1].h = img->comps[2].h = img->comps[0].h;
  img->comps[1].dx = img->comps[2].dx = img->comps[0].dx;
  img->comps[1].dy = img->comps[2].dy = img->comps[0].dy;
  img->color_space = OPJ_CLRSPC_SRGB;

  return true;

 out:
  free (r);
  free (g);
  free (b);
  return false;
}

static bool
sycc422_to_rgb (opj_image_t *img)
{
  int *d0, *d1, *d2, *r, *g, *b;
  const int *y, *cb, *cr;
  size_t maxw, maxh, max, offx, loopmaxw;
  int offset, upb;
  size_t i;

  upb = (int) img->comps[0].prec;
  offset = 1 <<(upb - 1);
  upb = (1 << upb) - 1;

  maxw = (size_t) img->comps[0].w;
  maxh = (size_t) img->comps[0].h;
  max = maxw * maxh;

  y = img->comps[0].data;
  cb = img->comps[1].data;
  cr = img->comps[2].data;

  d0 = r = (int *) malloc (sizeof (int) * max);
  d1 = g = (int *) malloc (sizeof (int) * max);
  d2 = b = (int *) malloc (sizeof (int) * max);

  if (r == NULL || g == NULL || b == NULL)
    {
      printf ("malloc() failed in sycc422_to_rgb()\n");
      goto out;
    }

  /* if img->x0 is odd, then first column shall use Cb/Cr = 0 */
  offx = img->x0 & 1U;
  loopmaxw = maxw - offx;

  for (i = 0U; i < maxh; ++i)
    {
      size_t j;

      if (offx > 0U)
        {
          sycc_to_rgb (offset, upb, *y, 0, 0, r, g, b);
          ++y;
          ++r;
          ++g;
          ++b;
        }

      for (j = 0U; j < (loopmaxw & ~(size_t) 1U); j += 2U)
        {
          sycc_to_rgb (offset, upb, *y, *cb, *cr, r, g, b);
          ++y;
          ++r;
          ++g;
          ++b;

          sycc_to_rgb (offset, upb, *y, *cb, *cr, r, g, b);
          ++y;
          ++r;
          ++g;
          ++b;
          ++cb;
          ++cr;
        }

      if (j < loopmaxw)
        {
          sycc_to_rgb (offset, upb, *y, *cb, *cr, r, g, b);
          ++y;
          ++r;
          ++g;
          ++b;
          ++cb;
          ++cr;
        }
    }

  free (img->comps[0].data);
  img->comps[0].data = d0;
  free (img->comps[1].data);
  img->comps[1].data = d1;
  free (img->comps[2].data);
  img->comps[2].data = d2;

  img->comps[1].w = img->comps[2].w = img->comps[0].w;
  img->comps[1].h = img->comps[2].h = img->comps[0].h;
  img->comps[1].dx = img->comps[2].dx = img->comps[0].dx;
  img->comps[1].dy = img->comps[2].dy = img->comps[0].dy;
  img->color_space = OPJ_CLRSPC_SRGB;

  return (true);

 out:
  free (r);
  free (g);
  free (b);
  return (false);
}

static bool
sycc444_to_rgb (opj_image_t *img)
{
  int *d0, *d1, *d2, *r, *g, *b;
  const int *y, *cb, *cr;
  size_t maxw, maxh, max, i;
  int offset, upb;

  upb = (int) img->comps[0].prec;
  offset = 1 << (upb - 1);
  upb = (1 << upb) - 1;

  maxw = (size_t) img->comps[0].w;
  maxh = (size_t) img->comps[0].h;
  max = maxw * maxh;

  y = img->comps[0].data;
  cb = img->comps[1].data;
  cr = img->comps[2].data;

  d0 = r = (int *) malloc(sizeof (int) * max);
  d1 = g = (int *) malloc(sizeof (int) * max);
  d2 = b = (int *) malloc(sizeof (int) * max);

  if (r == NULL || g == NULL || b == NULL)
    {
      printf("malloc() failed in sycc444_to_rgb()\n");
      goto out;
    }

  for (i = 0U; i < max; ++i)
    {
      sycc_to_rgb (offset, upb, *y, *cb, *cr, r, g, b);
      ++y;
      ++cb;
      ++cr;
      ++r;
      ++g;
      ++b;
    }

  free (img->comps[0].data);
  img->comps[0].data = d0;
  free (img->comps[1].data);
  img->comps[1].data = d1;
  free (img->comps[2].data);
  img->comps[2].data = d2;

  img->color_space = OPJ_CLRSPC_SRGB;
  return true;

 out:
  free (r);
  free (g);
  free (b);
  return false;
}

static bool
color_sycc_to_rgb (opj_image_t *img)
{
  if (img->numcomps < 3)
    {
      img->color_space = OPJ_CLRSPC_GRAY;
      return true;
    }
  else if ((img->comps[0].dx == 1) &&
           (img->comps[1].dx == 2) &&
           (img->comps[2].dx == 2) &&
           (img->comps[0].dy == 1) &&
           (img->comps[1].dy == 2) &&
           (img->comps[2].dy == 2))
    {
      /* horizontal and vertical sub-sample */
      return sycc420_to_rgb (img);
    }
  else if ((img->comps[0].dx == 1) &&
           (img->comps[1].dx == 2) &&
           (img->comps[2].dx == 2) &&
           (img->comps[0].dy == 1) &&
           (img->comps[1].dy == 1) &&
           (img->comps[2].dy == 1))
    {
      /* horizontal sub-sample only */
      return sycc422_to_rgb (img);
    }
  else if ((img->comps[0].dx == 1) &&
           (img->comps[1].dx == 1) &&
           (img->comps[2].dx == 1) &&
           (img->comps[0].dy == 1) &&
           (img->comps[1].dy == 1) &&
           (img->comps[2].dy == 1))
    {
      /* no sub-sample */
      return sycc444_to_rgb (img);
    }
  else
    {
      printf("Cannot convert in color_sycc_to_rgb()\n");
      return false;
    }
}
