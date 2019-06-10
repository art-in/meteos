if (process.env.NODE_ENV == 'production' && 'serviceWorker' in navigator) {
  // use the window load event to keep the page load performant
  window.addEventListener('load', () => {
    navigator.serviceWorker.register('sw-cache.js');
  });
}
