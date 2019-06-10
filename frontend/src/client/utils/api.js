import Sample from './Sample';
import moment from 'moment';

export async function getTime() {
  const response = await fetch('api/time');
  if (response.status !== 200) {
    throw Error('request failed');
  }
  return await response.text();
}

export async function getSamples({from, limit}) {
  let url = 'api/samples';

  const queryParams = [];

  if (limit) {
    queryParams.push(`limit=${limit}`);
  }

  if (from) {
    queryParams.push(
      `from=${encodeURIComponent(
        moment(from)
          .utcOffset(0)
          .format('YYYY-MM-DDTHH:mm:ss.SSS[Z]')
      )}`
    );
  }

  if (queryParams.length) {
    url += '?' + queryParams.join('&');
  }

  const response = await fetch(url);
  const data = await response.json();

  return data.map(s => new Sample(s.u, s.t, s.h, s.p, s.c));
}
