import {useState, useEffect} from 'react';

const LIMIT_FOR_LARGE_VIEWPORT = 300;
const LIMIT_FOR_MEDIUM_VIEWPORT = 150;
const LIMIT_FOR_SMALL_VIEWPORT = 75;

const largeViewport = matchMedia('(min-width: 769px)');
const mediumViewport = matchMedia('(min-width: 426px)');

/**
 * Gets samples limit to load and render on chart, depending on current
 * browser viewport size. Smaller viewport - lesser samples should be shown
 * to optimize rendering performance on mobiles.
 */
export default function useSamplesLimit() {
  const [samplesLimit, setSamplesLimit] = useState(getLimit());

  useEffect(() => {
    const onViewportChange = () => {
      const limit = getLimit();
      setSamplesLimit(limit);
    };

    mediumViewport.addEventListener('change', onViewportChange);
    largeViewport.addEventListener('change', onViewportChange);

    return () => {
      mediumViewport.removeEventListener('change', onViewportChange);
      largeViewport.removeEventListener('change', onViewportChange);
    };
  }, [setSamplesLimit]);

  return {samplesLimit};
}

function getLimit() {
  if (largeViewport.matches) {
    return LIMIT_FOR_LARGE_VIEWPORT;
  } else if (mediumViewport.matches) {
    return LIMIT_FOR_MEDIUM_VIEWPORT;
  } else {
    return LIMIT_FOR_SMALL_VIEWPORT;
  }
}
