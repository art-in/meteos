import Sample from './Sample';
import moment from 'moment';

export async function getSamples({from}) {
  let url = 'api/samples';
  if (from) {
    url += `?from=${encodeURIComponent(
      moment(from)
        .utcOffset(0)
        .format('YYYY-MM-DDTHH:mm:ss.SSS[Z]')
    )}`;
  }

  const response = await fetch(url);
  const data = await response.json();

  return data.map(s => new Sample(s.u, s.t, s.h, s.p, s.c));
}
