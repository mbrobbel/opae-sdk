// Copyright(c) 2018, Intel Corporation
//
// Redistribution  and  use  in source  and  binary  forms,  with  or  without
// modification, are permitted provided that the following conditions are met:
//
// * Redistributions of  source code  must retain the  above copyright notice,
//   this list of conditions and the following disclaimer.
// * Redistributions in binary form must reproduce the above copyright notice,
//   this list of conditions and the following disclaimer in the documentation
//   and/or other materials provided with the distribution.
// * Neither the name  of Intel Corporation  nor the names of its contributors
//   may be used to  endorse or promote  products derived  from this  software
//   without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING,  BUT NOT LIMITED TO,  THE
// IMPLIED WARRANTIES OF  MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED.  IN NO EVENT  SHALL THE COPYRIGHT OWNER  OR CONTRIBUTORS BE
// LIABLE  FOR  ANY  DIRECT,  INDIRECT,  INCIDENTAL,  SPECIAL,  EXEMPLARY,  OR
// CONSEQUENTIAL  DAMAGES  (INCLUDING,  BUT  NOT LIMITED  TO,  PROCUREMENT  OF
// SUBSTITUTE GOODS OR SERVICES;  LOSS OF USE,  DATA, OR PROFITS;  OR BUSINESS
// INTERRUPTION)  HOWEVER CAUSED  AND ON ANY THEORY  OF LIABILITY,  WHETHER IN
// CONTRACT,  STRICT LIABILITY,  OR TORT  (INCLUDING NEGLIGENCE  OR OTHERWISE)
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,  EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.

extern "C" {

#include <json-c/json.h>
#include <uuid/uuid.h>
#include "opae_int.h"
#include "types_int.h"
#include "metrics/vector.h"
#include "metrics/metrics_int.h"

#include "types_int.h"
#include "xfpga.h"
#include "intel-fpga.h"


}

#include <config.h>
#include <opae/fpga.h>

#include <array>
#include <cstdlib>
#include <map>
#include <memory>
#include <string>
#include <vector>
#include "gtest/gtest.h"
#include "test_system.h"
#include "xfpga.h"
#include "token_list_int.h"

#include "test_utils.h"

#include "intel-fpga.h"
#include "gtest/gtest.h"
#include "test_system.h"
#include <opae/access.h>
#include <opae/mmio.h>
#include <sys/mman.h>
#include <cstdarg>
#include <linux/ioctl.h>

using namespace opae::testing;


class metics_c_p : public ::testing::TestWithParam<std::string> {
protected:
	metics_c_p()
		: tokens_{ {nullptr, nullptr} },
		handle_(nullptr) {}

	virtual void SetUp() override {

		ASSERT_TRUE(test_platform::exists(GetParam()));
		platform_ = test_platform::get(GetParam());
		system_ = test_system::instance();
		system_->initialize();
		system_->prepare_syfs(platform_);

		ASSERT_EQ(xfpga_fpgaGetProperties(nullptr, &filter_), FPGA_OK);
		ASSERT_EQ(fpgaPropertiesSetObjectType(filter_, FPGA_DEVICE), FPGA_OK);
		ASSERT_EQ(xfpga_fpgaEnumerate(&filter_, 1, tokens_.data(), tokens_.size(),
			&num_matches_),
			FPGA_OK);

		ASSERT_EQ(xfpga_fpgaOpen(tokens_[0], &handle_, 0), FPGA_OK);
	}

	virtual void TearDown() override {
		EXPECT_EQ(fpgaDestroyProperties(&filter_), FPGA_OK);
		for (auto &t : tokens_) {
			if (t) {
				EXPECT_EQ(xfpga_fpgaDestroyToken(&t), FPGA_OK);
				t = nullptr;
			}
		}
		if (handle_ != nullptr) { EXPECT_EQ(xfpga_fpgaClose(handle_), FPGA_OK); }
		system_->finalize();
	}

	std::array<fpga_token, 2> tokens_;
	fpga_handle handle_;
	fpga_properties filter_;
	uint32_t num_matches_;
	test_platform platform_;
	test_system *system_;

};

/**
* @test    set_afu_userclock
* @brief   Tests: set_afu_userclock
* @details set_afu_userclock sets afu user clock
*          Returns FPGA_OK if parameters are valid. Returns
*          error code if invalid user clock or handle.
*/
TEST_P(metics_c_p, test_metric_01) {
	fpga_result result;

	fpga_handle handle_invalid;
	// get number of metrics
	uint64_t num_metrics;
	EXPECT_EQ(FPGA_OK, xfpga_fpgaGetNumMetrics(handle_, &num_metrics));


	// NULL input parameters
	EXPECT_NE(FPGA_OK, xfpga_fpgaGetNumMetrics(handle_, NULL));

	EXPECT_NE(FPGA_OK, xfpga_fpgaGetNumMetrics(NULL, &num_metrics));


	EXPECT_NE(FPGA_OK, xfpga_fpgaGetNumMetrics(handle_invalid, &num_metrics));


	struct _fpga_handle *_handle = (struct _fpga_handle *)handle_;

	int fddev = _handle->fddev;
	_handle->fddev = -1;
	EXPECT_NE(FPGA_OK, xfpga_fpgaGetNumMetrics(handle_, &num_metrics));


	_handle->fddev = fddev;

}

/**
* @test    set_afu_userclock
* @brief   Tests: set_afu_userclock
* @details set_afu_userclock sets afu user clock
*          Returns FPGA_OK if parameters are valid. Returns
*          error code if invalid user clock or handle.
*/
TEST_P(metics_c_p, test_metric_02) {
	fpga_result result;

	struct _fpga_handle *_handle = NULL;
	uint64_t num_metrics;
	EXPECT_EQ(FPGA_OK, xfpga_fpgaGetNumMetrics(handle_, &num_metrics));

	struct fpga_metric_info  *fpga_metric_info = (struct fpga_metric_info *) calloc(sizeof(struct fpga_metric_info), num_metrics);

	EXPECT_EQ(FPGA_OK, xfpga_fpgaGetMetricsInfo(handle_, fpga_metric_info, &num_metrics));


	EXPECT_NE(FPGA_OK, xfpga_fpgaGetMetricsInfo(NULL, fpga_metric_info, &num_metrics));

	EXPECT_NE(FPGA_OK, xfpga_fpgaGetMetricsInfo(handle_, NULL, &num_metrics));


	_handle = (struct _fpga_handle *)handle_;

	int fddev = _handle->fddev;
	_handle->fddev = -1;
	EXPECT_NE(FPGA_OK, xfpga_fpgaGetMetricsInfo(handle_, fpga_metric_info, &num_metrics));


	_handle->fddev = fddev;


	free(fpga_metric_info);
}

/**
* @test    set_afu_userclock
* @brief   Tests: set_afu_userclock
* @details set_afu_userclock sets afu user clock
*          Returns FPGA_OK if parameters are valid. Returns
*          error code if invalid user clock or handle.
*/
TEST_P(metics_c_p, test_metric_03) {
	fpga_result result;

	struct _fpga_handle *_handle = NULL;
	uint64_t num_metrics;



	uint64_t id_array[] = { 1, 5, 30, 35, 10 };


	struct fpga_metric  *metric_array = (struct fpga_metric *) calloc(sizeof(struct fpga_metric), 5);

	EXPECT_EQ(FPGA_OK, xfpga_fpgaGetMetricsByIndex(handle_, id_array, 5, metric_array));


	EXPECT_NE(FPGA_OK, xfpga_fpgaGetMetricsByIndex(NULL, id_array, 5, metric_array));

	EXPECT_NE(FPGA_OK, xfpga_fpgaGetMetricsByIndex(handle_, NULL, 5, metric_array));

	EXPECT_NE(FPGA_OK, xfpga_fpgaGetMetricsByIndex(handle_, id_array, 5, NULL));


	_handle = (struct _fpga_handle *)handle_;

	int fddev = _handle->fddev;
	_handle->fddev = -1;
	EXPECT_NE(FPGA_OK, xfpga_fpgaGetMetricsByIndex(handle_, id_array, 5, metric_array));


	_handle->fddev = fddev;


	free(metric_array);

}


/**
* @test    set_afu_userclock
* @brief   Tests: set_afu_userclock
* @details set_afu_userclock sets afu user clock
*          Returns FPGA_OK if parameters are valid. Returns
*          error code if invalid user clock or handle.
*/
TEST_P(metics_c_p, test_metric_04) {
	fpga_result result;

	struct _fpga_handle *_handle = NULL;


	char* metric_string[] = { "power_mgmt:consumed","performance:fabric:port0:mmio_read" };
	uint64_t  array_size = 2;

	struct fpga_metric  *metric_array_serach = (struct fpga_metric *) calloc(sizeof(struct fpga_metric), array_size);
	xfpga_fpgaGetMetricsByName(handle_,
		metric_string,
		array_size,
		metric_array_serach);




	char* metric_string_invalid[] = { "power_mgmtconsumed1","performance1:fabric:port0:mmio_read1" };


	struct fpga_metric*  metric_array_serach_invlaid = (struct fpga_metric *) calloc(sizeof(struct fpga_metric), array_size);
	xfpga_fpgaGetMetricsByName(handle_,
		metric_string_invalid,
		array_size,
		metric_array_serach_invlaid);

	free(metric_array_serach_invlaid);


	EXPECT_NE(FPGA_OK, xfpga_fpgaGetMetricsByName(NULL, metric_string, array_size, metric_array_serach));

	EXPECT_NE(FPGA_OK, xfpga_fpgaGetMetricsByName(handle_, NULL, array_size, metric_array_serach));

	EXPECT_NE(FPGA_OK, xfpga_fpgaGetMetricsByName(handle_, metric_string, array_size, NULL));

	EXPECT_NE(FPGA_OK, xfpga_fpgaGetMetricsByName(handle_, metric_string, 0, metric_array_serach));


	_handle = (struct _fpga_handle *)handle_;

	int fddev = _handle->fddev;
	_handle->fddev = -1;
	EXPECT_NE(FPGA_OK, xfpga_fpgaGetMetricsByName(handle_, metric_string, array_size, metric_array_serach));


	_handle->fddev = fddev;


	free(metric_array_serach);


}



INSTANTIATE_TEST_CASE_P(metics_c, metics_c_p, ::testing::ValuesIn(test_platform::keys(true)));
