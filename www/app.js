/*
 * Webserv demo interface
 * Modern client-side JavaScript for:
 * - CGI sorting request test
 * - Concurrent GET request test for non-blocking server handling
 *
 * Note: fetch() is asynchronous by nature. The code uses async/await instead
 * of .then() chains. Promise.all() is required in the concurrency test so the
 * HTTP requests are actually executed in parallel rather than one by one.
 */

'use strict';

/**
 * Sends a request to the CGI sorting endpoint and displays its response.
 */
const initCgiSortingDemo = () => {
  const form = document.querySelector('#sort-form');
  const input = document.querySelector('#sort-input');
  const output = document.querySelector('#sort-result');
  const button = form?.querySelector('button[type="submit"]');

  if (!form || !input || !output || !button) {
    return;
  }

  form.addEventListener('submit', async (event) => {
    event.preventDefault();

    const numbers = input.value.trim();

    if (!numbers) {
      output.textContent = 'Please enter comma-separated numbers, for example: 2,7,6,0.';
      return;
    }

    button.disabled = true;
    button.textContent = 'Running...';
    output.textContent = 'Sending request to the CGI endpoint...';

    try {
      const endpoint = `/sort/sort.cgi?numbers=${encodeURIComponent(numbers)}`;
      const response = await fetch(endpoint, {
        method: 'GET',
        cache: 'no-store'
      });

      if (!response.ok) {
        throw new Error(`HTTP ${response.status}`);
      }

      const result = await response.text();
      output.textContent = result || 'CGI completed with an empty response.';
    } catch (error) {
      output.textContent = `CGI request failed: ${error.message}`;
    } finally {
      button.disabled = false;
      button.textContent = 'Run CGI test';
    }
  });
};

/**
 * Sends one GET request to the server and records its response time.
 * A unique query string prevents the browser from serving a cached response.
 */
const sendTimedRequest = async (requestId) => {
  const startTime = performance.now();

  try {
    const response = await fetch(`/?concurrency_test=${requestId}`, {
      method: 'GET',
      cache: 'no-store'
    });

    return {
      ok: response.ok,
      status: response.status,
      duration: performance.now() - startTime
    };
  } catch (error) {
    return {
      ok: false,
      status: 'ERR',
      duration: performance.now() - startTime
    };
  }
};

/**
 * Displays the concurrency test statistics in the interface.
 */
const renderConcurrencyResults = (results, totalRequests, batchId) => {
  const testState = document.querySelector('#test-state');
  const successCount = document.querySelector('#success-count');
  const averageTime = document.querySelector('#average-time');
  const fastestTime = document.querySelector('#fastest-time');
  const slowestTime = document.querySelector('#slowest-time');
  const log = document.querySelector('#concurrency-log');

  const successfulResults = results.filter((result) => result.ok);
  const durations = results.map((result) => result.duration);
  // Group identical HTTP statuses to keep the output readable on small screens.
  // Example: 25 successful requests are displayed as "200 × 25" instead of a long list.
  const statusSummary = results.reduce((summary, result) => {
    const status = String(result.status);
    summary[status] = (summary[status] || 0) + 1;
    return summary;
  }, {});

  const statusOutput = Object.entries(statusSummary)
    .map(([status, count]) => `${status} × ${count}`)
    .join(' · ');
  const average = durations.reduce((sum, duration) => sum + duration, 0) / durations.length;
  const fastest = Math.min(...durations);
  const slowest = Math.max(...durations);

  successCount.textContent = `${successfulResults.length} / ${totalRequests}`;
  averageTime.textContent = `${average.toFixed(1)} ms`;
  fastestTime.textContent = `${fastest.toFixed(1)} ms`;
  slowestTime.textContent = `${slowest.toFixed(1)} ms`;
  log.textContent = `HTTP responses: ${statusOutput}
Endpoint: GET /
Requests: ${totalRequests}
Batch: ${batchId}`;

  const allRequestsSucceeded = successfulResults.length === totalRequests;
  testState.textContent = allRequestsSucceeded ? 'PASS' : 'PARTIAL';
  testState.className = allRequestsSucceeded
    ? 'rounded-md bg-emerald-400/10 px-2.5 py-1 font-mono text-xs text-emerald-300'
    : 'rounded-md bg-amber-400/10 px-2.5 py-1 font-mono text-xs text-amber-300';
};

/**
 * Launches multiple simultaneous GET requests to demonstrate how Webserv
 * handles several client responses through non-blocking I/O and poll().
 */
const initConcurrencyTest = () => {
  const form = document.querySelector('#concurrency-form');
  const requestCount = document.querySelector('#request-count');
  const button = document.querySelector('#concurrency-btn');
  const testState = document.querySelector('#test-state');
  const log = document.querySelector('#concurrency-log');

  if (!form || !requestCount || !button || !testState || !log) {
    return;
  }

  form.addEventListener('submit', async (event) => {
    event.preventDefault();

    const selectedCount = Number.parseInt(requestCount.value, 10) || 10;
    const totalRequests = Math.min(selectedCount, 25);
    const batchId = Date.now();

    button.disabled = true;
    button.textContent = 'Running...';
    testState.textContent = 'RUNNING';
    testState.className = 'rounded-md bg-sky-400/10 px-2.5 py-1 font-mono text-xs text-sky-300';
    log.textContent = `Dispatching ${totalRequests} parallel GET requests...`;

    try {
      // Build every request before awaiting them so they run concurrently.
      const requests = Array.from({ length: totalRequests }, (_, index) => {
        return sendTimedRequest(`${batchId}-${index}`);
      });

      // Waiting for all parallel requests is required to calculate statistics.
      const results = await Promise.all(requests);
      renderConcurrencyResults(results, totalRequests, batchId);
    } catch (error) {
      testState.textContent = 'ERROR';
      testState.className = 'rounded-md bg-rose-400/10 px-2.5 py-1 font-mono text-xs text-rose-300';
      log.textContent = `Concurrency test failed: ${error.message}`;
    } finally {
      button.disabled = false;
      button.textContent = 'Run concurrency test';
    }
  });
};

/**
 * Initialises all interactive demonstrations once the HTML has been loaded.
 */
document.addEventListener('DOMContentLoaded', () => {
  initCgiSortingDemo();
  initConcurrencyTest();
});
